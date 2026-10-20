#!/usr/bin/env python3
"""Exercise the actual yt-dlp plugin without contacting SoundCloud."""
from pathlib import Path
import importlib.util
import sys
sys.dont_write_bytecode = True
from urllib.parse import urlencode

r = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(r / 'soundcloud/vendor/yt-dlp-2026.08.19'))
spec = importlib.util.spec_from_file_location('nxt_sc_plugin', r / 'upstream/res/soundcloud/nxt/yt_dlp_plugins/extractor/nxt_soundcloud.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
ie = m.NxtSoundcloudIE()
seen = []

def track(i, **extra):
    return dict(id=i, title=f'Việt 한국어 日本語 中文 ไทย {i}', policy='ALLOW', streamable=True,
                duration=90000, user={'username': 'Uploader'}, permalink_url=f'https://soundcloud.com/u/track-{i}', **extra)

def run(kind, **args):
    return ie._real_extract('nxtsoundcloud:' + kind + '?' + urlencode(args or {'offset': 0}))

def api(url, *args, **kwargs):
    seen.append((url, kwargs.get('query', {})))
    if url.endswith('search/tracks'):
        offset = kwargs['query']['offset']
        return {'collection': [track(i) for i in range(offset + 1, offset + 22)]}
    if url.endswith('search/playlists'):
        return {'collection': [dict(track(i), kind='playlist') for i in range(1, 4)]}
    if url.endswith('playlists/1'):
        return {'id': 1, 'title': 'Fixture', 'tracks': [track(i) if i < 30 else {'id': i} for i in range(1, 53)]}
    if url.endswith('playlists/2'):
        return {'id':2,'title':'All blocked','tracks':[dict(track(2),policy='BLOCK')]}
    if url.endswith('playlists/3'):
        return {'id':3,'title':'Partial metadata','tracks':[{'id':31}]}
    if url.endswith('/tracks'):
        return [track(int(i)) for i in kwargs['query']['ids'].split(',')]
    if 'resolve?' in url:
        return dict(track(1), kind='playlist')
    if url.endswith('tracks/1'):
        return track(1)
    if url.endswith('tracks/2'):
        return dict(track(2), policy='SNIP')
    raise AssertionError(url)

ie._call_api = api
a = run('search', query='Việt 日本語', offset=0)
b = run('search', query='Việt 日本語', offset=20)
assert len(a['entries']) == 20 and a['nxt_has_next']
assert a['entries'][-1]['id'] == '20' and b['entries'][0]['id'] == '21'
assert seen[1][1]['offset'] == 20 and seen[1][1]['limit'] == 21 and seen[1][1]['access']=='playable'
sets=run('search_playlists', query='fixture')['entries'];assert [x['id'] for x in sets]==['1','3'];assert all(x['nxt_playlist'] for x in sets)
ids = []
for offset in (0, 20, 40):
    result = run('playlist_tracks', resource='1', offset=offset)
    ids += [row['id'] for row in result['entries']]
    assert result['nxt_has_next'] == (offset < 40)
    assert all(row['title'].startswith('Việt') for row in result['entries'])
assert ids == list(map(str, range(1, 53)))
assert run('resolve', query='https://soundcloud.com/u/sets/demo')['nxt_kind'] == 'search_playlists'
for url in ('http://soundcloud.com/u/t', 'https://evil.test/t', 'https://soundcloud.com/u/t/s-private',
            'https://soundcloud.com:444/u/t', 'https://soundcloud.com/u/t?secret_token=s-x'):
    assert not ie.public_link(url)
for extra in ({'policy': 'BLOCK'}, {'policy': 'SNIP'}, {'sharing': 'private'}, {'access': 'preview'}, {'streamable': False}, {'monetization_model': 'SUB_HIGH_TIER'}, {'policy': None, 'streamable': None}):
    assert ie.row(dict(track(1), **extra)) is None
ie._extract_info_dict = lambda info: {'id': '1', 'formats': [
    {'format_id': 'hls_aac_160k', 'url': 'https://cf-hls-media.sndcdn.com/full'},
    {'format_id': 'hls_mp3_preview', 'url': 'https://cf-hls-media.sndcdn.com/preview/'},
    {'format_id': 'hls_mp3', 'url': 'https://cf-hls-media.sndcdn.com/playlist/0/30/x'},
    {'format_id': 'hls-aes_aac', 'url': 'https://cf-hls-media.sndcdn.com/encrypted'},
]}
assert len(run('play', resource='1')['formats']) == 1
for monetization in ('BLACKBOX', 'AD_SUPPORTED', None):
    monetized = dict(track(1), policy='MONETIZE', monetization_model=monetization)
    assert ie.permitted(monetized) and ie.row(monetized)
    # Exercise the actual play route with fresh metadata, not only row filtering.
    original_api = ie._call_api
    ie._call_api = lambda *args, **kwargs: monetized
    assert len(run('play', resource='1')['formats']) == 1
    ie._call_api = original_api
for extra in ({'policy': 'BLOCK'}, {'policy': 'SNIP'}, {'sharing': 'private'},
              {'access': 'preview'}, {'access': 'blocked'}, {'streamable': False},
              {'monetization_model': 'SUB_HIGH_TIER'}):
    restricted = dict(track(1), policy='MONETIZE')
    restricted.update(extra)
    assert not ie.permitted(restricted)
try:
    run('play', resource='2')
    raise AssertionError('preview track was accepted')
except m.ExtractorError as e:
    assert 'track_restricted' in str(e)
print('PASS real yt-dlp plugin: UTF-8 search, playlist search/resolve, 52-row pagination + metadata hydration, guest-only URLs, private/blocked/preview/encrypted rejection.')
