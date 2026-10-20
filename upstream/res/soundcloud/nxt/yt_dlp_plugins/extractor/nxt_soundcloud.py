# SPDX-License-Identifier: GPL-3.0-only
"""Small native-player adapter using yt-dlp's maintained SoundCloud extractor.

Only public guest resources are accepted. No cookies, user grants, API secret,
download, or preview fallback. Custom fields survive --dump-single-json.
"""
import re
from urllib.parse import parse_qs, urlsplit

from yt_dlp.extractor.soundcloud import SoundcloudBaseIE
from yt_dlp.utils import ExtractorError


class NxtSoundcloudIE(SoundcloudBaseIE):
    IE_NAME = 'nxt:soundcloud'
    _VALID_URL = r'nxtsoundcloud:(?P<id>search|search_playlists|playlist_tracks|resolve|play)\?.+'

    @staticmethod
    def public_link(url):
        p = urlsplit(url)
        return (p.scheme == 'https' and p.hostname in (
            'soundcloud.com', 'www.soundcloud.com', 'on.soundcloud.com')
            and p.port in (None, 443) and not p.username and not p.password
            and not parse_qs(p.query).get('secret_token')
            and not re.search(r'/s-[A-Za-z0-9]+(?:/|$)', p.path))

    @staticmethod
    def permitted(info):
        # MONETIZE describes an ad/monetization policy, not a Go+ block.
        # Guest web responses often omit `access`, including playable tracks.
        return ((info.get('access') == 'playable' or (info.get('policy') in ('ALLOW', 'MONETIZE') and info.get('streamable') is True))
                and info.get('sharing', 'public') == 'public'
                and info.get('policy') not in ('SNIP', 'BLOCK')
                and info.get('access') not in ('preview', 'blocked')
                and info.get('streamable') is not False
                and info.get('monetization_model') != 'SUB_HIGH_TIER')

    def row(self, info, playlist=False):
        track_id = str(info.get('id', ''))
        if not track_id.isascii() or not track_id.isdigit() or len(track_id) > 32:
            return None
        if info.get('sharing', 'public') != 'public' or (not playlist and not self.permitted(info)):
            return None
        uploader = (info.get('user') or {}).get('username') or 'SoundCloud'
        permalink = info.get('permalink_url') or ''
        artist = ((info.get('publisher_metadata') or {}).get('artist') or uploader)
        return {
            '_type': 'url', 'ie_key': 'SoundcloudPlaylist' if playlist else 'Soundcloud',
            'url': permalink or f'{self._API_V2_BASE}{"playlists" if playlist else "tracks"}/{track_id}',
            'id': track_id, 'title': info.get('title') or f'SoundCloud {track_id}',
            'uploader': uploader, 'artist': artist,
            'duration': (info.get('duration') or 0) / 1000,
            'nxt_cover': info.get('artwork_url') or '',
            'nxt_permalink': permalink, 'nxt_playlist': playlist,
        }

    def _real_extract(self, url):
        kind = self._match_id(url)
        args = parse_qs(urlsplit(url).query)
        query = args.get('query', [''])[0]
        offset = int(args.get('offset', ['0'])[0])
        if offset < 0 or offset > 20000 or offset % 20:
            raise ExtractorError('NXT:page_invalid', expected=True)
        resource = args.get('resource', [''])[0]
        if kind in ('play', 'playlist_tracks'):
            if not re.fullmatch(r'[0-9]{1,32}', resource):
                raise ExtractorError('NXT:track_invalid', expected=True)
            endpoint = f'{self._API_V2_BASE}{"tracks" if kind == "play" else "playlists"}/{resource}'
            info = self._call_api(endpoint, resource, 'Reading public resource')
        elif kind == 'resolve':
            if not self.public_link(query):
                raise ExtractorError('NXT:link_invalid', expected=True)
            if urlsplit(query).hostname == 'on.soundcloud.com':
                response = self._request_webpage(query, 'link', 'Opening SoundCloud short link')
                query = response.url
                if not self.public_link(query):
                    raise ExtractorError('NXT:link_invalid', expected=True)
            info = self._call_api(self._resolv_url(query), 'link', 'Resolving public link')
        elif kind in ('search', 'search_playlists'):
            if not query.strip() or len(query) > 500:
                raise ExtractorError('NXT:query_invalid', expected=True)
            playlist = kind == 'search_playlists'
            info = self._call_api(
                self._API_V2_BASE + ('search/playlists' if playlist else 'search/tracks'),
                query, 'Searching SoundCloud', query={
                    'q': query, 'limit': 21, 'offset': offset, 'linked_partitioning': 1,
                    **({} if playlist else {'access': 'playable', 'filter': 'public'}),
                })
            collection = info.get('collection') or []
            if playlist:
                checked = []
                for row in collection[:20]:
                    if not row or row.get('sharing', 'public') != 'public':
                        continue
                    # Search summaries can omit track permissions. Resolve the
                    # playlist, hydrate partial IDs, and admit only a set with
                    # a full-length public track. Requests remain bounded.
                    pid = str(row.get('id', ''))
                    if not re.fullmatch(r'[0-9]{1,32}', pid):
                        continue
                    try:
                        full = row if row.get('tracks') else self._call_api(
                            self._API_V2_BASE + 'playlists/' + pid, pid, 'Checking playlist access')
                        tracks = self.hydrate(full.get('tracks') or [], pid, 0)
                        if any(self.permitted(t) and t.get('title') for t in tracks):
                            checked.append(row)
                    except ExtractorError:
                        continue
                collection_rows = checked
            else:
                collection_rows = collection[:20]
            rows = [self.row(row, playlist) for row in collection_rows if row]
            return self.playlist_result(
                [row for row in rows if row], f'{kind}-{offset}', 'SoundCloud',
                nxt_kind=kind, nxt_has_next=bool(len(collection) > 20 or info.get('next_href')))
        else:
            raise ExtractorError('NXT:command_invalid', expected=True)

        if kind == 'play':
            if not self.permitted(info):
                raise ExtractorError('NXT:track_restricted', expected=True)
            # Avoid the original-download lookup; the player streams AAC/MP3.
            info['downloadable'] = False
            result = self._extract_info_dict(info)
            result['formats'] = [f for f in result.get('formats', []) if
                                 'preview' not in f.get('format_id', '').lower()
                                 and '/preview/' not in f.get('url', '').lower()
                                 and not re.search(r'/(?:preview|playlist)/0/30/', f.get('url', ''))
                                 and f.get('preference') != -10
                                 and 'aes' not in f.get('format_id', '').lower()]
            if not result['formats']:
                raise ExtractorError('NXT:track_restricted', expected=True)
            return result

        if kind == 'resolve':
            playlist = info.get('kind') == 'playlist'
            row = self.row(info, playlist)
            if not row:
                raise ExtractorError('NXT:track_restricted', expected=True)
            return self.playlist_result([row], 'resolve', 'SoundCloud',
                                        nxt_kind='search_playlists' if playlist else 'search',
                                        nxt_has_next=False)

        if info.get('sharing', 'public') != 'public':
            raise ExtractorError('NXT:track_restricted', expected=True)
        tracks = info.get('tracks') or []
        page = self.hydrate(tracks, resource, offset)
        rows = [self.row(t) for t in page if t]
        return self.playlist_result([row for row in rows if row], resource,
                                    info.get('title') or 'SoundCloud playlist',
                                    nxt_kind=kind, nxt_has_next=len(tracks) > offset + 20)

    def hydrate(self, tracks, resource, offset):
        page = tracks[offset:offset + 20]
        missing = [str(t['id']) for t in page if t and not t.get('title')
                   and re.fullmatch(r'[0-9]{1,32}', str(t.get('id', '')))]
        if missing:
            full = self._call_api(self._API_V2_BASE + 'tracks', resource,
                                  'Reading playlist metadata', query={'ids': ','.join(missing)})
            by_id = {str(t['id']): t for t in full if t and t.get('id') is not None}
            page = [by_id.get(str(t.get('id')), t) for t in page if t]
        return [t for t in page if t and t.get('title') and self.permitted(t)]
