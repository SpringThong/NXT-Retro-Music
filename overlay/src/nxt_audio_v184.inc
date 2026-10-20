/* SPDX-License-Identifier: GPL-3.0-only */
static ModuleExitReason dac_name_settings(DisplayContext*d){
 if(!NxtHardware_hasDAC()){Toast_show(Nxt_text("Connect USB DAC first","Hãy cắm DAC USB trước"),2200);return MODULE_EXIT_TO_MENU;}
 int sel=0;const char*rows[]={Nxt_text("Use USB descriptor","Dùng tên USB tự khai báo"),"TRN Black Pearl",Nxt_text("Enter display name","Nhập tên hiển thị")};
 NxtPick p=Nxt_pick(d,Nxt_text("Name this DAC","Đặt tên DAC này"),rows,3,&sel,Nxt_text("A Save · B Back","A Lưu · B Về"));if(p.action==9)return MODULE_EXIT_QUIT;
 if(p.action==1){char*custom=sel==2?Keyboard_open(Nxt_text("DAC display name","Tên hiển thị DAC"),96):NULL;const char*name=sel==0?"":sel==1?"TRN Black Pearl":custom;
 if(name)Toast_show(NxtHardware_setDACName(name)?Nxt_text("DAC name saved for this USB identity","Đã nhớ tên cho nhận dạng USB này"):Nxt_text("Cannot save DAC name","Không lưu được tên DAC"),2200);free(custom);}
 return MODULE_EXIT_TO_MENU;
}
static ModuleExitReason dsd_output(DisplayContext*d){
 int selected=nxt.dsd_output==2?0:nxt.dsd_output+1;
 const char*rows[]={Nxt_text("Automatic: Native, then confirmed DoP\nOriginal DSD; never converts to PCM","Tự động: Native, sau đó DoP đã xác nhận\nGiữ DSD gốc; không chuyển thành PCM"),Nxt_text("Native DSD only\nRequires ROM USB driver DSD support","Chỉ Native DSD\nCần driver USB của ROM hỗ trợ DSD"),Nxt_text("DoP only\nFor a DAC supporting DSD over PCM","Chỉ DoP\nDành cho DAC hỗ trợ DSD over PCM"),Nxt_text("Status and compatibility","Trạng thái và tương thích")};
 NxtPick p=Nxt_pick(d,Nxt_text("DSD USB output","Đầu ra DSD USB"),rows,4,&selected,Nxt_text("A Select · B Back","A Chọn · B Về"));if(p.action==9)return MODULE_EXIT_QUIT;if(p.action!=1)return MODULE_EXIT_TO_MENU;
 if(selected==3){const char*docs[]={NxtBit_status(),NxtBit_error(),Nxt_text("Local raw DSF/DFF mono/stereo. DST compression and SACD ISO are not supported.","DSF/DFF DSD thô mono/stereo. Không hỗ trợ nén DST và SACD ISO."),Nxt_text("Native depends on ROM driver support. DoP also preserves original DSD bits; it needs confirmed DAC support. PCM capability alone cannot confirm DoP.","Native phụ thuộc driver ROM. DoP cũng giữ bit DSD gốc, cần DAC xác nhận hỗ trợ; khả năng PCM không đủ để xác nhận DoP."),Nxt_text("DoP carrier: DSD64 = 176400 Hz; DSD128 = 352800 Hz; DSD256 = 705600 Hz. A DAC may accept a lower DSD tier through DoP than through Native.","Tần số truyền DoP: DSD64 = 176400 Hz; DSD128 = 352800 Hz; DSD256 = 705600 Hz. Mức DSD qua DoP có thể thấp hơn Native."),Nxt_text("No DSD to speaker/Bluetooth and no PCM conversion. Removing the DAC pauses DSD.","Không phát DSD qua loa/Bluetooth, không chuyển PCM. Rút DAC sẽ tạm dừng DSD."),Nxt_text("About: START exports app log, Y exports USB identity and ROM formats in the device log.","Thông tin: START xuất log app, Y xuất nhận dạng USB và định dạng ROM vào log thiết bị.")};return Nxt_document(d,"DSD",docs,7);}
 int mode=selected==0?2:selected==1?0:1;
 if(mode){int confirm=0;const char*items[]={Nxt_text("Cancel","Hủy"),Nxt_text("My DAC supports DoP\nEnable original DSD over an exact PCM carrier","DAC của tôi hỗ trợ DoP\nBật DSD gốc trong khung truyền PCM chính xác")};NxtPick c=Nxt_pick(d,Nxt_text("Confirm DoP support for your DAC","Xác nhận DAC hỗ trợ DoP"),items,2,&confirm,Nxt_text("A Confirm · B Back","A Xác nhận · B Về"));if(c.action==9)return MODULE_EXIT_QUIT;if(c.action!=1||confirm!=1)return MODULE_EXIT_TO_MENU;}
 nxt.dsd_output=mode;NxtBit_setDSDMode(mode);Nxt_save();return MODULE_EXIT_TO_MENU;
}

static ModuleExitReason audio_output(DisplayContext *d){
    const char *rows[]={Nxt_text("Speaker / wired headphones","Loa máy / tai nghe dây"),"Bluetooth","USB DAC"};int selected=nxt.output<3?nxt.output:0;
    NxtPick p=Nxt_pick(d,Nxt_text("Audio output","Đầu ra âm thanh"),rows,3,&selected,Nxt_text("A Select · B Back","A Chọn · B Về"));
    if(p.action==9)return MODULE_EXIT_QUIT;
    if(p.action==1){if(NxtHardware_select(selected)){nxt.output=selected;Nxt_save();}else Toast_show(Nxt_text("Connect the output in system settings first","Kết nối đầu ra trong cài đặt hệ thống trước"),2500);}
    return MODULE_EXIT_TO_MENU;
}
static ModuleExitReason audio_mode(DisplayContext *d){
    int selected=nxt.audio_mode;
    for(;;){const char *rows[]={Nxt_text("Normal\nEQ/DSP, shared output and software volume","Thường\nEQ/DSP, đầu ra chia sẻ và âm lượng phần mềm"),Nxt_text("Bit-Perfect · USB DAC\nNative lossless PCM; saved EQ is retained","Bit-Perfect · DAC USB\nPCM lossless nguyên bản; giữ EQ đã lưu"),Nxt_text("Automatic\nUSB DAC: try Bit-Perfect; other outputs: Normal","Tự động\nDAC USB: thử Bit-Perfect; đầu ra khác: Thường"),Nxt_text("Status and usage","Trạng thái và cách dùng")};
        NxtPick p=Nxt_pickLive(d,Nxt_text("Sound mode","Chế độ âm thanh"),rows,4,&selected,Nxt_text("A Select · B Back","A Chọn · B Về"));
        if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
        if(p.action!=1)continue;
        if(selected<3){NxtAudio_setMode(selected);Toast_show(NxtAudio_modeName(selected),2000);}
        else{const char *docs[]={nxt.bit_perfect?NxtBit_status():Nxt_text("Normal mode","Chế độ Thường"),NxtBit_error(),Nxt_text("USB DAC: compatible local lossless PCM uses its original rate/channels. Online sources temporarily use Normal.","DAC USB: PCM lossless nội bộ tương thích giữ đúng Hz/kênh. Nguồn online tạm dùng Thường."),Nxt_text("Native PCM bypasses effects. Use volume buttons on the device or DAC; software-only gain uses Normal.","PCM nguyên bản bỏ qua hiệu ứng. Chỉnh âm lượng trên máy hoặc DAC; tăng giảm bằng phần mềm dùng Thường."),Nxt_text("Manual Bit-Perfect pauses on incompatible PCM. Automatic uses Normal for incompatible sources and when USB is removed.","Bit-Perfect thủ công dừng khi PCM không phù hợp. Tự động dùng Thường nếu nguồn không phù hợp hoặc USB bị rút."),Nxt_text("Digital transport precision is different from the DAC's analog quality. Verify the physical DAC on your device.","Độ chính xác dữ liệu số khác chất lượng analog của DAC. Kiểm tra DAC thật trên máy của bạn.")};if(Nxt_document(d,"Bit-Perfect",docs,6)==MODULE_EXIT_QUIT)return MODULE_EXIT_QUIT;}
    }
}
static ModuleExitReason audio_settings(DisplayContext *d){
    int selected=0;
    for(;;){char mode[180];snprintf(mode,sizeof(mode),"%s\t%s",Nxt_text("Sound mode","Chế độ âm thanh"),nxt.bit_perfect?"Bit-Perfect":Nxt_text("Normal","Thường"));
        const char *rows[]={Nxt_text("Playback\nResume, transition, timer and background","Cách phát\nNghe tiếp, chuyển bài, hẹn giờ và phát nền"),Nxt_text("Sound & output\nSpeaker, 3.5 mm, Bluetooth, DAC and sound mode","Âm thanh & đầu ra\nLoa, 3,5 mm, Bluetooth, DAC và chế độ âm thanh")};
        NxtPick p=Nxt_pickLive(d,Nxt_text("Playback & audio","Phát nhạc & âm thanh"),rows,2,&selected,Nxt_text("A Open · B Back","A Mở · B Về"));
        if(p.action==2||p.action==9)return p.action==9?MODULE_EXIT_QUIT:MODULE_EXIT_TO_MENU;
        if(p.action!=1)continue;
        ModuleExitReason r=selected==0?setting_group(d,3):setting_group(d,1);
        if(r==MODULE_EXIT_QUIT)return r;
    }
}
