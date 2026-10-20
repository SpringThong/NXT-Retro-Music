#!/usr/bin/env python3
"""Opt-in real public SoundCloud: production IPC, decoder, seek and playlist.
Never saves audio or signed stream URLs. Runs a host helper or QEMU wrapper.
"""
from pathlib import Path
import json, os, queue, socket, struct, subprocess, sys, tempfile, threading, time

r = Path(__file__).resolve().parents[1]
binary = Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory() as td:
    data = Path(td) / 'soundcloud'
    data.mkdir()
    left, right = socket.socketpair(socket.AF_UNIX, socket.SOCK_SEQPACKET)
    def fd():
        os.dup2(right.fileno(), 3)
    child = subprocess.Popen([str(binary)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, pass_fds=tuple({right.fileno(), 3}), preexec_fn=fd,
        env={**os.environ, 'NXT_SOUNDCLOUD_PCM_FD': '3', 'NXT_SOUNDCLOUD_CREDENTIALS': str(data/'credentials.json')})
    right.close()
    events = queue.Queue()
    def read_events():
        for line in child.stdout:
            events.put(json.loads(line))
    threading.Thread(target=read_events, daemon=True).start()
    stats = {}
    lock = threading.Lock()
    def pcm():
        while True:
            try:
                packet = left.recv(8208)
            except OSError:
                break
            if not packet:
                break
            epoch, count, rate, eof = struct.unpack('<IIII', packet[:16])
            assert len(packet) == 16 + count*4
            samples = struct.unpack('<'+'f'*count, packet[16:]) if count else ()
            with lock:
                row = stats.setdefault(epoch, {'frames': 0, 'peak': 0, 'rate': rate, 'eof': False})
                row['frames'] += count//2
                row['peak'] = max(row['peak'], max(map(abs, samples), default=0))
                row['eof'] |= bool(eof)
    threading.Thread(target=pcm, daemon=True).start()
    def send(value):
        child.stdin.write(json.dumps(value, ensure_ascii=False)+'\n')
        child.stdin.flush()
    def receive(kind, timeout=75):
        until = time.monotonic()+timeout
        while True:
            v = events.get(timeout=max(.01, until-time.monotonic()))
            if v['event'] == 'error':
                raise AssertionError(v)
            if v['event'] == kind:
                return v
    try:
        assert receive('ready')['credentials_required'] is False
        send({'cmd':'login'})
        assert receive('logged_in')['mode'] == 'public_unofficial'
        send({'cmd':'search','query':'lofi','request_id':1})
        tracks = receive('results')
        assert len(tracks['items']) == 20
        track = tracks['items'][0]
        print('Live search passed', file=sys.stderr, flush=True)
        send({'cmd':'play','uri':track['uri'],'stream_id':1})
        first = receive('playing')
        print('Live playback decoded '+first['codec'], file=sys.stderr, flush=True)
        until = time.monotonic()+30
        while stats.get(1, {}).get('frames', 0) < 44100 and time.monotonic() < until:
            time.sleep(.05)
        assert stats[1]['frames'] >= 44100 and stats[1]['peak'] > .001
        send({'cmd':'seek','uri':track['uri'],'stream_id':2,'position_ms':30000})
        seek = receive('playing')
        print('Live seek passed', file=sys.stderr, flush=True)
        assert seek['stream_id'] == 2 and seek['position_ms'] == 30000
        until = time.monotonic()+30
        while stats.get(2, {}).get('frames', 0) < 44100 and time.monotonic() < until:
            time.sleep(.05)
        assert stats[2]['frames'] >= 44100
        send({'cmd':'stop'})
        send({'cmd':'search_playlists','query':'lofi','request_id':2})
        playlists = receive('results')
        print('Live playlist search passed', file=sys.stderr, flush=True)
        assert playlists['items'] and playlists['items'][0]['id'].startswith('soundcloud:playlists:')
        send({'cmd':'playlist_tracks','id':playlists['items'][0]['id'],'request_id':3})
        page = receive('results')
        assert page['items'] and all(row['title'] for row in page['items'])
        send({'cmd':'resolve','query':track['permalink'],'request_id':4})
        resolved = receive('results')
        assert resolved['items'][0]['uri'] == track['uri']
        assert not (data/'credentials.json').exists()
        report={'key_required':False,'search_count':len(tracks['items']),
                'codec':first['codec'],'sample_rate':first['sample_rate'],
                'decoded_pcm':stats,'seek_ms':30000,'playlist_count':len(playlists['items']),
                'playlist_tracks_count':len(page['items']),'resolve':True,
                'hardware_tested':False}
        print(json.dumps(report, indent=2))
    finally:
        send({'cmd':'quit'})
        try:
            child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            child.kill(); child.wait()
        left.close()
    assert child.returncode == 0
