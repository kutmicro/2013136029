// 김태훈 아두이노 최종 블루투스 스피커 

#include <SPI.h>              // SPI통신을 위한 라이브러리 추가
#include <SdFat.h>            // SDFat 라이브러리 추가
#include <SdFatUtil.h>        // SDFatUtil 라이브러리 추가
#include <SFEMP3Shield.h>     // MP3 Shield 라이브러리 추가

SdFat sd;                      // SD카드를 읽기위한 변수 설정
SFEMP3Shield MP3player;        // 플레이를 위한 변수 설정.
char trackName[] = "track001.mp3";

void setup() {                 // setup 루프

  Serial3.begin(9600);        // 블루투스 시리얼속도를 9600으로 설정.
  Serial.begin(115200);       // MP3 shield를 시리얼 모니터에 나타내기 위해 115200으로 설정.
  if(!sd.begin(SD_SEL, SPI_FULL_SPEED)) sd.initErrorHalt(); // sd card를 초기화해서 저장
  
  MP3player.begin();
}

void loop() {                         // loop문 

  if(Serial3.available()) {     // 블루투스에서 보낸 값이 있을 경우
    parse_menu(Serial3.read()); // parse_menu에 입력값을 전달
  }
  delay(100);                    // 0.1초 대기
}

void parse_menu(byte key_command) {          // 명령 값을 byte로 받아와 전달

  Serial.write(key_command);                // 명령 값을 씀

  if(key_command == 's') {
     MP3player.stopTrack();                              // 플레이 도중 s가 입력되면 플레이를 멈춤.
   }
  else if(key_command == 'r') {
    for(int i = 1; i<= 10; i++) {
      int trackNumber = random(1,10);
      MP3player.playTrack(trackNumber);
    }
  }
  else if(key_command == 't') {
    for(int i = 1; i <= 10; i++) {
      MP3player.playTrack(i);
    }
  }
   else if(key_command >= '1' && key_command <= '9') {   // 입력 값중 1부터 9사이의 값이 입력되면.
     key_command = key_command - 48;                     // 아스키 코드로 입력된 값을 실수로 변환함.

#if USE_MULTIPLE_CARDS
    sd.chvol();                                          // sd카드 초기 볼륨을 설정해줌.
#endif
    MP3player.playTrack(key_command);           // MP3 트랙에 입력 값을 전달
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
    if(key_command == '>') {
      if(playspeed >= 254) { // 범위를 체크해줌.
        playspeed = 5;       // 최고 빠르기를 5단계라고 설정
      } else {
        playspeed += 1;      // 빠르기를 한단계씩 증가
      }
    } else {
      if(playspeed == 0) {   // 범위를 체크해줌.
        playspeed = 0;       // 최저 빠르기를 0으로 설정
      } else {
        playspeed -= 1;      // 빠르기가 증가되어있을 시 1단계씩 감소
      }
    }
    MP3player.setPlaySpeed(playspeed);  // 입력이 끝났을 시 빠르기 재정의
  } else if(key_command == 'p') {       // p가 입력되었을 시
    if( MP3player.getState() == playback) { // 상태가 플레이 중이면
      MP3player.pauseMusic();               // 음악 일시중지
    } else if( MP3player.getState() == paused_playback) { // 상태가 일시중지 중이면
      MP3player.resumeMusic();                            // 음악 다시 재생
    } 
  }  
}


