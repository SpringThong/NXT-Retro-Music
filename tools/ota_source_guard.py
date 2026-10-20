"""Do not publish installers with empty, fixture, obsolete or known wrong sources."""
from pathlib import Path
from urllib.parse import urlsplit
import os

def publisher_source(path: Path) -> str:
    if os.environ.get('NXT_OTA_QA_FIXTURE') == '1':
        return 'https://drive.google.com/file/d/PUBLIC_MANIFEST_ID/view'
    lines = path.read_text().splitlines() if path.is_file() else []
    url = lines[0].strip() if lines else ''
    parsed = urlsplit(url)
    rejected = ('REPLACE_', 'THAY_', 'PUBLIC_MANIFEST_ID', '1heNcSJ7Um70Z_82cS5atY093ObUZgNR-', '1RrUF1iqdMsNEwtp6ekRh9oz17gndNOzT')
    if parsed.scheme != 'https' or not parsed.netloc or parsed.username or parsed.password or '/folders/' in parsed.path or any(s in url for s in rejected):
        raise RuntimeError('Official OTA update.json source is missing or invalid: ' + path.name)
    return url
