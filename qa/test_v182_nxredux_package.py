#!/usr/bin/env python3
"""Verify release scope and byte preservation against the recovered v1.8.2 source."""
from pathlib import Path
import hashlib, json, os, zipfile

R = Path(__file__).resolve().parents[1]
BASE = Path(os.environ.get('NXT_BASELINE_SOURCE', str(R.parents[1] / 'nx-redux-input/NXT_Retro_Music_v1.8.2_Source.zip')))
PREFIX = 'Tools/NXT Retro Music.pak/'
INSTALLER = R / 'dist/nxredux-release/NXT_Retro_Music_v1.8.2_NXRedux.zip'
KIT = R / 'dist/nxredux-release/NXT_OTA_Google_Drive_v1.8.2_NXRedux.zip'

def digest(path):
    with path.open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()

def archive_digest(z, name):
    with z.open(name) as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()

def main():
    assert BASE.is_file(), BASE
    unchanged_assets = unchanged_backends = changed_locales = 0
    baseline_root = 'NXT_Retro_Music_v1.8.2_Source/'
    help_keys = {
        'Timer ends: save position and close NXT, including background playback',
        'NX Redux controls screen-off and deep sleep after NXT exits'
    }
    with zipfile.ZipFile(BASE) as old, zipfile.ZipFile(INSTALLER) as app:
        assert app.testzip() is None
        assert len(app.namelist()) == len(set(app.namelist()))
        cfg = json.loads(app.read(PREFIX + 'config.json'))
        assert (cfg['rom'], cfg['version'], cfg['device']) == ('nxredux', '1.8.2', 'trimui-brick-pro')
        for item in old.infolist():
            name = item.filename.removeprefix(baseline_root)
            if item.is_dir() or not name.startswith('upstream/'):
                continue
            relative = name.removeprefix('upstream/')
            if relative.startswith('res/'):
                prior = old.read(item.filename)
                current = app.read(PREFIX + relative)
                if prior != current:
                    assert relative.startswith('res/locales/') and relative.endswith('.json'), relative
                    before, after = json.loads(prior), json.loads(current)
                    assert set(after) - set(before) == help_keys, relative
                    assert {k: v for k, v in after.items() if k not in help_keys} == before, relative
                    assert all(after[k].strip() for k in help_keys)
                    changed_locales += 1
                else:
                    unchanged_assets += 1
            elif relative.startswith('bin/tg5040/') and relative not in {
                'bin/tg5040/musicplayer.elf', 'bin/tg5040/libmsettings.so'
            }:
                assert archive_digest(old, item.filename) == archive_digest(app, PREFIX + relative), relative
                unchanged_backends += 1
        assert changed_locales == 14
        for relative in ['nxredux/bin/tg5040/musicplayer.elf', 'nxredux/bin/tg5040/libnxt-nxredux-settings.so']:
            installed = PREFIX + relative.removeprefix('nxredux/')
            assert archive_digest(app, installed) == digest(R / relative)
            assert app.getinfo(installed).external_attr >> 16 & 0o111
        assert PREFIX + 'bin/tg5040/libmsettings.so' not in app.namelist()
        assert not any(n.startswith(('.system/', '.userdata/', 'Apps/', 'Tools/tg5040/')) for n in app.namelist())
        previous_frontends = {}
        for relative in ['stock/bin/tg5040/musicplayer.elf', 'upstream/bin/tg5040/musicplayer.elf']:
            prior = archive_digest(old, baseline_root + relative)
            assert digest(R / relative) == prior, relative
            previous_frontends[relative] = prior
    with zipfile.ZipFile(KIT) as kit:
        assert kit.testzip() is None
        inner = next(n for n in kit.namelist() if n.endswith('/' + INSTALLER.name))
        assert archive_digest(kit, inner) == digest(INSTALLER)
    report = {
        'exit': 0, 'version': '1.8.2', 'rom': 'nxredux',
        'baseline_source_sha256': digest(BASE),
        'installer_sha256': digest(INSTALLER), 'OTA_kit_sha256': digest(KIT),
        'unchanged_UI_assets': unchanged_assets, 'unchanged_runtime_files': unchanged_backends,
        'locale_changes_only_two_timer_help_entries': changed_locales,
        'flat_Tools_path_and_executable_modes': True,
        'installed_firmware_settings_library_not_replaced': True,
        'exact_installer_in_OTA_kit': True,
        'historical_frontends_preserved': previous_frontends,
    }
    (R / 'logs/nxredux-package-test.json').write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))

if __name__ == '__main__':
    main()
