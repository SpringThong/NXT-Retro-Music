ModuleExitReason Nxt_help(DisplayContext *d){int selected=0;for(;;){const char *sections[]={Nxt_text("Local music & playlists","Nhạc trong thẻ & playlist"),Nxt_text("Online music","Nghe nhạc online"),Nxt_text("Personal profiles","Hồ sơ cá nhân"),Nxt_text("Buttons & shortcuts","Các phím & phím tắt"),Nxt_text("Themes, artwork & LEDs","Giao diện, ảnh album & LED"),Nxt_text("Copy files over Wi-Fi","Chép file qua Wi-Fi"),Nxt_text("Languages & keyboards","Ngôn ngữ & bàn phím"),Nxt_text("Performance & background playback","Hiệu năng & nghe nhạc nền")};NxtPick p=Nxt_pick(d,Nxt_text("Usage guide","Hướng dẫn sử dụng"),sections,8,&selected,Nxt_text("A Open section · B Back","A Mở mục · B Về"));if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;if(p.action!=1)continue;
 const char *rows[24];int n=0;char mapped[16][200];switch(selected){
 case 0:
 rows[n++]=Nxt_text("Choose your music folder in Settings → Library & files. Saved tracks open immediately; new music is checked in the background on launch and after Wi-Fi copies. Library START offers a new-music check and full metadata rebuild.","Chọn thư mục nhạc ở Cài đặt → Thư viện & tệp. Danh sách đã lưu mở ngay; tự kiểm tra nhạc mới ở nền khi mở app và sau chép Wi-Fi. START trong Thư viện có kiểm tra nhạc mới và quét lại toàn bộ thông tin.");
 rows[n++]=Nxt_text("Open Library, choose a song and press A. X in Now playing opens the complete current queue.","Mở Thư viện, chọn bài rồi bấm A. X ở Đang phát mở đầy đủ danh sách đang nghe.");
 rows[n++]=Nxt_text("Queue: A plays, X removes, L1/R1 changes order, Y saves a local M3U8 playlist.","Danh sách: A phát, X bỏ bài, L1/R1 đổi thứ tự, Y lưu playlist M3U8 của nhạc trên thẻ.");break;
 case 1:
 rows[n++]=Nxt_text("Connect Wi-Fi in your firmware settings, then open Online music.","Kết nối Wi-Fi trong cài đặt hệ thống, rồi mở Nhạc online.");
 rows[n++]=Nxt_text("Spotify: scan the pairing QR and approve your account. Playback requires Spotify Premium.","Spotify: quét QR ghép nối và xác nhận tài khoản. Phát nhạc cần Spotify Premium.");
 rows[n++]=Nxt_text("Lyric translation uses the app language. Enable it in Playback settings, START quick options or START in full-screen lyrics.","Dịch lời theo ngôn ngữ app. Bật ở Cách phát, tùy chọn START hoặc START trong lời toàn màn hình.");
 rows[n++]=Nxt_text("SoundCloud plays eligible public tracks. Drive supports public folder and audio links.","SoundCloud phát các bài công khai được phép nghe. Drive hỗ trợ link thư mục và file nhạc công khai.");
 rows[n++]=Nxt_text("Personal → For you offers real local history and online suggestions from today's choices.","Cá nhân → Dành cho bạn dùng lịch sử trên máy và gợi ý online theo lựa chọn hôm nay.");break;
 case 2:
 rows[n++]=Nxt_text("Select a listener first, then open their saved song and position in Now Playing. Mood, genre and country are chosen only in Personal.","Chọn hồ sơ trước, rồi mở bài và vị trí nghe dở ở Đang phát. Tâm trạng, thể loại và quốc gia chỉ chọn trong Cá nhân.");
 rows[n++]=Nxt_text("Your avatar is shared with the startup chooser. Personal → Profile → Edit profile edits it.","Avatar cá nhân đồng bộ với màn chọn hồ sơ. Sửa ở Cá nhân → Hồ sơ → Sửa hồ sơ.");
 rows[n++]=Nxt_text("Choose from 300 icons, 180 3D emoji, letters or an SD photo. Each profile keeps its own preferences.","Chọn 300 icon, 180 emoji 3D, chữ hoặc ảnh trên thẻ. Mỗi hồ sơ lưu tùy chọn riêng.");
 rows[n++]=Nxt_text("Change today's listening choices in Personal → Profile.","Đổi lựa chọn nghe hôm nay ở Cá nhân → Hồ sơ.");break;
 case 3:
 for(int i=0;i<16;i++){snprintf(mapped[n],sizeof(mapped[n]),"%s · %s",Nxt_keyNames[i],NxtControl_name(nxt.playing_keys[i]));rows[n]=mapped[n];n++;}
 rows[n++]=Nxt_text("SELECT + Up/Down: volume. SELECT + Left/Right: brightness. SELECT + START: lock/unlock.","SELECT + Lên/Xuống: âm lượng. SELECT + Trái/Phải: độ sáng. SELECT + START: khóa/mở khóa.");
 rows[n++]=Nxt_text("START opens shuffle, repeat and sleep timer. The player shows the remaining timer and queue count.","START mở ngẫu nhiên, lặp lại và hẹn giờ. Đang phát hiển thị giờ còn lại và số bài trong danh sách.");break;
 case 4:
 rows[n++]=Nxt_text("Personal → Profile → Personal appearance → Theme collection has 50 choices including Original NXT. Y previews; A applies; B cancels.","Cá nhân → Hồ sơ → Giao diện cá nhân → Bộ sưu tập có 50 giao diện gồm NXT gốc. Y xem trước; A áp dụng; B hủy.");
 rows[n++]=Nxt_text("Original theme options adjusts the original NXT wallpaper, album/vinyl, fonts and appearance.","Tùy chọn giao diện gốc chỉnh hình nền, ảnh album/đĩa than, font và bố cục NXT gốc.");
 rows[n++]=Nxt_text("All album images fit their frames. Original NXT retains your wallpaper, RGB and album/vinyl options.","Ảnh album vừa khung. Original NXT giữ tùy chọn hình nền, RGB và ảnh album/đĩa xoay của bạn.");
 rows[n++]=Nxt_text("LED color, mode and effect changes apply immediately. You do not need to turn LEDs off and on.","Màu, chế độ và hiệu ứng LED đổi ngay. Không cần tắt rồi bật lại LED.");break;
 case 5:
 rows[n++]=Nxt_text("Wi-Fi: open Settings → Library & files → Wi-Fi file transfer. Open the displayed URL on the same network.","Wi-Fi: mở Cài đặt → Thư viện & file → Chuyển file qua Wi-Fi. Mở địa chỉ hiển thị trên cùng mạng.");
 rows[n++]=Nxt_text("Enter the six-digit PIN and select files. The browser shows progress. B stops the server; X cancels the current file.","Nhập PIN sáu số rồi chọn file. Trình duyệt hiện tiến độ. B đóng máy chủ; X hủy file đang chép.");
 rows[n++]=Nxt_text("Uploads stream into the chosen music folder. Existing files are preserved; partial uploads are removed.","File được chép vào thư mục nhạc đã chọn. Giữ file đã có; xóa file chưa chép xong.");break;
 case 6:
 rows[n++]=Nxt_text("First launch defaults to English. Settings → Language selects one of fourteen languages.","Lần đầu mặc định tiếng Anh. Cài đặt → Ngôn ngữ chọn một trong 14 ngôn ngữ.");
 rows[n++]=Nxt_text("The keyboard opens in the chosen language. Y switches layout. Hold A to choose accented alternatives.","Bàn phím mở theo ngôn ngữ đã chọn. Y đổi bố cục. Giữ A để chọn ký tự có dấu.");
 rows[n++]=Nxt_text("Korean uses two-set composition. Japanese uses Romaji/Kana. Chinese uses Pinyin; Space/X opens offline candidates.","Tiếng Hàn ghép chữ hai bộ. Nhật dùng Romaji/Kana. Trung dùng Pinyin; phím cách/X mở các chữ gợi ý offline.");
 rows[n++]=Nxt_text("L1: Shift. R1: Caps. SELECT: Done. URL and PIN dialogs start with the Latin keyboard.","L1: Shift. R1: khóa hoa. SELECT: xong. Mục nhập URL và PIN bắt đầu bằng bàn phím Latin.");break;
 case 7:
 rows[n++]=Nxt_text("Settings → Device performance → Performance mode offers Battery saver, Balanced and Performance. Balanced is the default.","Cài đặt → Hiệu năng thiết bị → Chế độ hiệu năng có Tiết kiệm pin, Cân bằng và Hiệu năng. Mặc định là Cân bằng.");
 rows[n++]=Nxt_text("NextUI modes adjust supported CPUFreq caps and visual refresh; ROM CPU policy is restored on exit. Stock / NX Redux keep firmware CPU control. Audio processing is unchanged.","NextUI đổi giới hạn CPUFreq hỗ trợ và nhịp vẽ, trả chính sách ROM khi thoát. Stock / NX Redux giữ CPU do ROM quản lý. Giữ xử lý âm thanh.");
 rows[n++]=Nxt_text("Screen-off and hidden sessions skip visual rendering. Audio and button handling continue independently.","Tắt màn hình và ẩn app sẽ ngừng vẽ giao diện. Âm thanh và xử lý phím tiếp tục độc lập.");
 rows[n++]=Nxt_text("Turn animations off or lower LED brightness if you prefer less visual activity. Device performance shows actual sensor readings.","Tắt chuyển động hoặc giảm sáng LED nếu muốn ít hiệu ứng. Hiệu năng thiết bị hiện số đo từ cảm biến thật.");
 rows[n++]=Nxt_text("Physical-device heat, battery life and USB compatibility still depend on firmware, DAC and your music files.","Nhiệt độ, thời lượng pin và tương thích USB trên máy phụ thuộc hệ thống, DAC và file nhạc của bạn.");break;
 }if(Nxt_document(d,sections[selected],rows,n)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;
}}
