/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef NXT_RELEASE_NOTES_H
#define NXT_RELEASE_NOTES_H
#include "nxt.h"
#include <string.h>
/* Current release stays separate from the historical information-page layout. */
static inline const char *NxtRelease_intro(const char *title){
 if(!title||strcmp(title,Nxt_text("What’s new · v" NXT_VERSION,"Những thay đổi · v" NXT_VERSION)))return NULL;
 return Nxt_text("v1.10.2: QR uses authenticated client-library services for playlists, liked songs, saved albums and favorites; no developer ID, redirect URI or PC JSON is required for this mode. Track search uses Spotify context results; artist/album search derives matches from those tracks, and playlist search checks your own library. B keeps the v1.10.1 behavior. Bounded pages, batched metadata and small session caches retain the existing sharp themes and audio pipeline. This protocol needs testing with your real Spotify Premium account; all three original OTA sources remain.",
 "v1.10.2: QR dùng dịch vụ thư viện của phiên đã xác thực để đọc playlist, bài đã thích, album đã lưu và yêu thích; chế độ này không yêu cầu tạo ID, URI chuyển hướng hay JSON PC. Tìm bài dùng kết quả ngữ cảnh Spotify; tìm nghệ sĩ/album lấy từ các bài khớp, tìm playlist trong thư viện của bạn. Giữ nút B của v1.10.1. Phân trang giới hạn, lấy thông tin theo nhóm và cache nhỏ; giữ độ nét theme và bộ phát cũ. Cần thử giao thức này với tài khoản Spotify Premium thật. Giữ ba link OTA chính thức.");
}
#endif
