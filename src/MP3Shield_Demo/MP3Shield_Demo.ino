/*  김태훈 아두이노 최종 블루투스 스피커 
 */

#include <SPI.h>              // SPI통신을 위한 라이브러리 추가
#include <SdFat.h>            // SDFat 라이브러리 추가
#include <SdFatUtil.h>        // SDFatUtil 라이브러리 추가
#include <SFEMP3Shield.h>     // MP3 Shield 라이브러리 추가

// Below is not needed if interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_Timer1
  #include <TimerOne.h>
#elif defined(USE_MP3_REFILL_MEANS) && USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer
  #include <SimpleTimer.h>
#endif

SdFat sd;                      // SD카드를 읽기위한 변수 설정
SFEMP3Shield MP3player;        // 플레이를 위한 변수 설정.

void setup() {                 // setup 루프

  Serial3.begin(9600);        // 블루투스 시리얼속도를 9600으로 설정.

  uint8_t result;              // 기능 테스트를 위한 result 변수 설정.(부호 없는 char을 나타내기 위해 uint8_t를 사용하였음.)
  
  Serial.begin(115200);       // MP3 shield를 시리얼 모니터에 나타내기 위해 115200으로 설정.

  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_FULL_SPEED)) sd.initErrorHalt();
  // depending upon your SdCard environment, SPI_HAVE_SPEED may work better.
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");

  //Initialize the MP3 Player Shield
  result = MP3player.begin();
  //check result, see readme for error codes.
  if(result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    if( result == 6 ) {
      Serial.println(F("Warning: patch file not found, skipping.")); // can be removed for space, if needed.
      Serial.println(F("Use the \"d\" command to verify SdCard can be read")); // can be removed for space, if needed.
    }
  }

#if defined(__BIOFEEDBACK_MEGA__) // or other reasons, of your choosing.
  // Typically not used by most shields, hence commented out.
  Serial.println(F("Applying ADMixer patch."));
  if(MP3player.ADMixerLoad("admxster.053") == 0) {
    Serial.println(F("Setting ADMixer Volume."));
    MP3player.ADMixerVol(-3);
  }
#endif

  help();
}

void loop() {                         // loop문 

// Below is only needed if not interrupt driven. Safe to remove if not using.
#if defined(USE_MP3_REFILL_MEANS) \
    && ( (USE_MP3_REFILL_MEANS == USE_MP3_SimpleTimer) \
    ||   (USE_MP3_REFILL_MEANS == USE_MP3_Polled)      )

  MP3player.available();
#endif

  if(Serial3.available()) {     // 블루투스에서 보낸 값이 있을 경우
    parse_menu(Serial3.read()); // parse_menu에 입력값을 전달
  }

  delay(100);                    // 0.1초 대기
}

void parse_menu(byte key_command) {          // 명령 값을 byte로 받아와 전달

  uint8_t result;                            // 기능 테스트를 위한 result 변수 설정.(부호 없는 char을 나타내기 위해 uint8_t를 사용하였음.)
  char title[30];                            // 현재의 파일 핸들로부터 Title을 추출하는 버퍼
  char artist[30];                           // 현재 파일 핸들에서 아티스트 이름을 추출하는 버퍼
  char album[30];                            // 현재의 파일 핸들에서 앨범 이름을 추출하는 버퍼

  Serial.print(F("Received command: "));    // 명령 값 전달
  Serial.write(key_command);                // 명령 값을 씀
  Serial.println(F(" "));

  if(key_command == 's') {
     Serial.println(F("Stopping"));          
     MP3player.stopTrack();                              // 플레이 도중 s가 입력되면 플레이를 멈춤.
   } 
   else if(key_command >= '1' && key_command <= '9') {   // 입력 값중 1부터 9사이의 값이 입력되면.
     key_command = key_command - 48;                     // 아스키 코드로 입력된 값을 실수로 변환함.

#if USE_MULTIPLE_CARDS
    sd.chvol();                                          // sd카드 초기 볼륨을 설정해줌.
#endif
    result = MP3player.playTrack(key_command);           // MP3 트랙에 입력 값을 전달하여 result에 저장


    if(result != 0) {                                    // result값을 체크하여 0이 아니면 Error code 발생.
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    } 
    else {
      Serial.println(F("Playing:"));                    // 값이 0일 경우 플레이 

        MP3player.trackTitle((char*)&title);
        MP3player.trackArtist((char*)&artist);             // 다음 함수와 인자를 사용하여 트랙 정보를 시리얼 모니터에 나타내기 위한 변수
        MP3player.trackAlbum((char*)&album);               // 함수는 요청 된 정보를 추출하여 배열에 저장해줌.

        // 트랙에 정보를 출력해줌.
        Serial.write((byte*)&title, 30);                  // char* 형으로 입력된 값을 byte* 형으로 형변환 시켜서 출력 
        Serial.println();
        Serial.print(F("by:  "));
        Serial.write((byte*)&artist, 30);
        Serial.println();
        Serial.print(F("Album:  "));
        Serial.write((byte*)&album, 30);
        Serial.println();
     }
  } 
  else if((key_command == '-') || (key_command == '+')) { // 입력 값이 '-' 나 '+' 가 입력되었을 경우
    union twobyte mp3_vol;                                // key_command 기존 변수를 생성 왼쪽과 오른쪽의 워드에 더블 바이트로 정의
    mp3_vol.word = MP3player.getVolume();                 // int16_t의 왼쪽과 오른쪽에 double uint8_t를 반환해줌.

    if(key_command == '-') {                              // 커맨드에 -가 입력된 경우.
      if(mp3_vol.byte[1] >= 254) {                        // 범위를 체크해줌. ( 2부터 254까지 최소 소리크기와 최대 소리크기를 제한해줌. )
        mp3_vol.byte[1] = 254;                            // 크기가 254보다 큰 경우 254를 저장함.
      } else {
        mp3_vol.byte[1] += 2;                             // 그 이외의 값인 경우 -가 입력될 때마다 byte값을 2씩 증가시켜 저장함.
      }
    } else {                                              // 커맨드에 +가 입력된 경우.
      if(mp3_vol.byte[1] <= 2) {                          // 범위를 체크해줌. ( 2부터 254까지 최소 소리크기와 최대 소리크기를 제한해줌. )
        mp3_vol.byte[1] = 2;                              // 크기가 2보다 작을 경우 2를 저장해줌.
      } else {                                       
        mp3_vol.byte[1] -= 2;                             // 그 이외의 값인 경우 +가 입력될 때마다 byte값을 2씩 감소시켜 저장함.
      }
    }
    // byte[1] 의 값을 왼쪽과 오른쪽으로 균등 한 균형을 가정하여 입력을 넣어줌.
    
    MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]); // 새로운 볼륨을 커밋해줌.
    Serial.print(F("Volume changed to -"));
    Serial.print(mp3_vol.byte[1]>>1, 1);                  // 왼쪽 시프트를 사용하여 값을 출력.
    Serial.println(F("[dB]"));
  } 
  //  >와 < 가 입력되었을 시 음악의 재생 속도를 변경시켜줌.
  else if((key_command == '>') || (key_command == '<')) {
    uint16_t playspeed = MP3player.getPlaySpeed();               // playspeed라는 변수정의해줌.
    if(key_command == '>') { // note dB is negative
      // assume equal balance and use byte[1] for math
      if(playspeed >= 254) { // range check
        playspeed = 5;
      } else {
        playspeed += 1; // keep it simpler with whole dB's
      }
    } else {
      if(playspeed == 0) { // range check
        playspeed = 0;
      } else {
        playspeed -= 1;
      }
    }
    MP3player.setPlaySpeed(playspeed); // commit new playspeed
    Serial.print(F("playspeed to "));
    Serial.println(playspeed, DEC);

  /* Alterativly, you could call a track by it's file name by using playMP3(filename);
  But you must stick to 8.1 filenames, only 8 characters long, and 3 for the extension */
  } else if(key_command == 'f' || key_command == 'F') {
    uint32_t offset = 0;
    if (key_command == 'F') {
      offset = 2000;
    }

    //create a string with the filename
    char trackName[] = "track001.mp3";

#if USE_MULTIPLE_CARDS
    sd.chvol(); // assign desired sdcard's volume.
#endif
    //tell the MP3 Shield to play that file
    result = MP3player.playMP3(trackName, offset);
    //check result, see readme for error codes.
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to play track"));
    }

  /* Display the file on the SdCard */
  } else if(key_command == 'd') {
    if(!MP3player.isPlaying()) {
      // prevent root.ls when playing, something locks the dump. but keeps playing.
      // yes, I have tried another unique instance with same results.
      // something about SdFat and its 500byte cache.
      Serial.println(F("Files found (name date time size):"));
      sd.ls(LS_R | LS_DATE | LS_SIZE);
    } else {
      Serial.println(F("Busy Playing Files, try again later."));
    }

  } else if(key_command == 'p') {
    if( MP3player.getState() == playback) {
      MP3player.pauseMusic();
      Serial.println(F("Pausing"));
    } else if( MP3player.getState() == paused_playback) {
      MP3player.resumeMusic();
      Serial.println(F("Resuming"));
    } else {
      Serial.println(F("Not Playing!"));
    }

  } else if(key_command == 't') {
    int8_t teststate = MP3player.enableTestSineWave(126);
    if(teststate == -1) {
      Serial.println(F("Un-Available while playing music or chip in reset."));
    } else if(teststate == 1) {
      Serial.println(F("Enabling Test Sine Wave"));
    } else if(teststate == 2) {
      MP3player.disableTestSineWave();
      Serial.println(F("Disabling Test Sine Wave"));
    }
  }
  else if(key_command == 'r') {
    MP3player.resumeMusic(2000);

  } else if(key_command == 'g') {
    int32_t offset_ms = 20000; // Note this is just an example, try your own number.
    Serial.print(F("jumping to "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skipTo(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }

  } else if(key_command == 'k') {
    int32_t offset_ms = -1000; // Note this is just an example, try your own number.
    Serial.print(F("moving = "));
    Serial.print(offset_ms, DEC);
    Serial.println(F("[milliseconds]"));
    result = MP3player.skip(offset_ms);
    if(result != 0) {
      Serial.print(F("Error code: "));
      Serial.print(result);
      Serial.println(F(" when trying to skip track"));
    }
   } 
  else if(key_command == 'h') {
    help();
  }

void help() {
  Serial.println(F("Arduino SFEMP3Shield Library Example:"));
  Serial.println(F(" courtesy of Bill Porter & Michael P. Flaga"));
  Serial.println(F("COMMANDS:"));
  Serial.println(F(" [1-9] to play a track"));
  Serial.println(F(" [f] play track001.mp3 by filename example"));
  Serial.println(F(" [F] same as [f] but with initial skip of 2 second"));
  Serial.println(F(" [s] to stop playing"));
  Serial.println(F(" [d] display directory of SdCard"));
  Serial.println(F(" [+ or -] to change volume"));
  Serial.println(F(" [> or <] to increment or decrement play speed by 1 factor"));
  Serial.println(F(" [i] retrieve current audio information (partial list)"));
  Serial.println(F(" [p] to pause."));
  Serial.println(F(" [t] to toggle sine wave test"));
  Serial.println(F(" [S] Show State of Device."));
  Serial.println(F(" [b] Play a MIDI File Beep"));
#if !defined(__AVR_ATmega32U4__)
  Serial.println(F(" [e] increment Spatial EarSpeaker, default is 0, wraps after 4"));
  Serial.println(F(" [m] perform memory test. reset is needed after to recover."));
  Serial.println(F(" [M] Toggle between Mono and Stereo Output."));
  Serial.println(F(" [g] Skip to a predetermined offset of ms in current track."));
  Serial.println(F(" [k] Skip a predetermined number of ms in current track."));
  Serial.println(F(" [r] resumes play from 2s from begin of file"));
  Serial.println(F(" [R] Resets and initializes VS10xx chip."));
  Serial.println(F(" [O] turns OFF the VS10xx into low power reset."));
  Serial.println(F(" [o] turns ON the VS10xx out of low power reset."));
  Serial.println(F(" [D] to toggle SM_DIFF between inphase and differential output"));
  Serial.println(F(" [V] Enable VU meter Test."));
  Serial.println(F(" [B] Increament bass frequency by 10Hz"));
  Serial.println(F(" [C] Increament bass amplitude by 1dB"));
  Serial.println(F(" [T] Increament treble frequency by 1000Hz"));
  Serial.println(F(" [E] Increament treble amplitude by 1dB"));
#endif
  Serial.println(F(" [h] this help"));
}


