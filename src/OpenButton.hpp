#ifndef __OPENBUTTON_H__
#define __OPENBUTTON_H__
#include <Arduino.h>

/** Class that returns button results */
class OpenButton{
  private:
    static const uint32_t BTN_MAX = 32;
    static const uint32_t DBLCLK_MS = 200;
    static const uint32_t W_CUR = 0;
    static const uint32_t W_PRE = 1;
    static const uint32_t W_CNT = 2;
  public:
  OpenButton(){
      m_btnWorkArr=NULL;
  };
  ~OpenButton(){
    if(m_btnWorkArr!=NULL){
      free(m_btnWorkArr);
      m_btnWorkArr=NULL;
    }
};

  /** a 1st time of the button pressed */
  bool onPress(int8_t _btnId){ return m_btnWorkArr[_btnId].onPress; }
  /** a 1st time of the button released */
  bool onRelease(int8_t _btnId){ return m_btnWorkArr[_btnId].onRelease; }
  /** a 1st time of the button double clicked */
  bool onDblClk(int8_t _btnId){ return m_btnWorkArr[_btnId].onDblClk; }
  /** button was hold over _millis and released. */
  bool onHold(int8_t _btnId, uint32_t _millis){ return onRelease(_btnId) && wasHold(_btnId,_millis); }

  /** button is on */
  bool isOn(int8_t _btnId){ return m_btnWorkArr[_btnId].val[W_CUR]==1; }
  /** button is pressed and hold over _millis. */
  bool isHold(int8_t _btnId, uint32_t _millis){ return m_btnWorkArr[_btnId].holdMillis>=_millis; }
  /** button was hold over _millis before release. */
  bool wasHold(int8_t _btnId, uint32_t _millis){ return m_btnWorkArr[_btnId].previousHoldMillis>=_millis; }

  /** Setup button system.
   *  _maxBtn: Maximum number of Buttons (max:32)
   *  _times: Number of iterations for chattering (default: 3, max:255)*/
  void Setup(uint8_t _maxBtn=3, uint8_t _times=3){
    m_btnNum = (_maxBtn<BTN_MAX)?_maxBtn:BTN_MAX;
    m_btnWorkArr = (BtnWork*)malloc(sizeof(BtnWork)*m_btnNum);
    m_calcNum = _times;
    m_calcTotal = 0;
    for(int i=0;i<m_btnNum;++i){
      m_btnWorkArr[i].gpio = -1;
      m_btnWorkArr[i].val[W_CUR]=0;
      m_btnWorkArr[i].val[W_PRE]=0;
      m_btnWorkArr[i].val[W_CNT]=0;
    }
    m_currentMillis = millis();
    m_deltaMillis = 0;
  }

  /** Add a button with gpio.
   * return: sucsess=buttonID, -1 on failure */
  int8_t AddButton(int32_t _gpio){
    for(int8_t btnId=0;btnId<m_btnNum;btnId++){
      if(m_btnWorkArr[btnId].gpio<0){
        m_btnWorkArr[btnId].reset();
        m_btnWorkArr[btnId].gpio = _gpio;
        pinMode(_gpio, INPUT_PULLUP);
        return btnId;
      }
    }
    return -1;
  }

  /** Remove a button of _btnId.
   * return: true when success */
    bool RemoveButton(int8_t _btnId){
    if(m_btnWorkArr[_btnId].gpio>=0){
      pinMode(m_btnWorkArr[_btnId].gpio, INPUT);
      m_btnWorkArr[_btnId].reset();
      return true;
    }
    return false;
  }

  /** Call this in loop()
   * return: Button status (Up to the first 32 buttons)
   * The same result will be returned for times of iterations. */
  uint32_t Update(uint32_t _deltaMillis){
      uint32_t results = calcTrig(_deltaMillis);
      m_currentMillis = millis();
      return results;
  };

  /** Call this if you need to update this with a different time interval
   * return: Button status (Up to the first 32 buttons)
   * The same result will be returned for times of iterations. */
  uint32_t Update(){
      uint32_t delta = millis()-m_currentMillis;
      return Update(delta);
  };

  /** Get the number of remaining buttons available
   * return : number of buttons */
  int8_t GetRemainButtonNum(){
      int8_t cnt = 0;
      for(int8_t i=0;i<m_btnNum;++i)
        if(m_btnWorkArr[i].gpio<0)
          cnt++;
      return cnt;
  }
  private:
  struct BtnWork{
      int32_t gpio=-1;
      int32_t val[4]={0,0,0,0}; // raw,pre,onCnt,dummy
      bool onRelease=false;
      bool onPress=false;
      bool onDblClk=false;
      unt32_t openMillis=0;
      unt32_t holdMillis=0;
      unt32_t previousHoldMillis=0;
      int32_t multiClickCnt=0;
      void reset(){
        gpio = -1;
        val[W_CUR] = 0;
        val[W_PRE] = 0;
        val[W_CNT] = 0;
        onRelease=false;
        onPress=false;
        onDblClk=false;
        openMillis=0;
        holdMillis=0;
        previousHoldMillis=0;
        multiClickCnt=0;
      }
  } _BtnWork;
  BtnWork* m_btnWorkArr;
  int8_t m_btnNum;
  uint32_t m_currentMillis;
  uint32_t m_deltaMillis;
  int8_t m_calcTotal;
  int8_t m_calcNum;

  /** Calculate button status
   * return: bit flags of Button (Up to the first 32 buttons) */
  uint32_t calcTrig(uint32_t _deltaMillis){
    for(int i=0;i<m_btnNum;++i){
      if(m_btnWorkArr[i].gpio>=0){
        bool isOn = digitalRead(m_btnWorkArr[i].gpio)==0;
        m_btnWorkArr[i].val[W_CNT] += isOn ? 1 : 0;
        if(isOn){
          m_btnWorkArr[i].holdMillis += _deltaMillis; // hold time
        }
        else{
          m_btnWorkArr[i].openMillis += _deltaMillis; // open time
        }
        m_btnWorkArr[i].onRelease = 0; // trigger only once
        m_btnWorkArr[i].onPress = 0; // trigger only once
        m_btnWorkArr[i].onDblClk = 0; // trigger only once
      }
    }
    m_calcTotal++;
    if(m_calcTotal>=m_calcNum){
      m_calcTotal=0;
      for(int i=0;i<m_btnNum;++i){
          if(m_btnWorkArr[i].gpio>=0){
            updateStatus(i,_deltaMillis);
          }
      }
    }
    int resultNum = ((m_btnNum<=32) ? m_btnNum : 32);
    uint32_t result = 0;
    for(int i=0;i<resultNum;++i){
      if(m_btnWorkArr[i].gpio>=0){
        result |= ((uint32_t)(m_btnWorkArr[i].val[W_CUR]&1) << i);
      }
    }
    return result;
  }

  void updateStatus(int _btnIdx, uint32_t _delta){
    int32_t pre = m_btnWorkArr[_btnIdx].val[W_CUR] ;
    int32_t cur = (m_btnWorkArr[_btnIdx].val[W_CNT]>m_calcNum/2)?1:0;
    BtnWork* btn = &m_btnWorkArr[_btnIdx];
    btn->val[W_PRE] = pre;
    btn->val[W_CUR] = cur;
    btn->val[W_CNT] = 0;
    btn->onRelease = (pre==1 && cur==0);
    btn->onPress = (pre==0 && cur==1);
    if(btn->openMillis>=DBLCLK_MS){
      btn->multiClickCnt = 0;
      btn->onDblClk = false;
    }
    if(btn->onRelease){
      btn->previousHoldMillis = btn->holdMillis;
      btn->holdMillis = 0;
      btn->openMillis = 0;
      if(btn->previousHoldMillis<DBLCLK_MS){
        btn->multiClickCnt++;
        if(btn->multiClickCnt>=2){
          btn->onDblClk = true;
          btn->multiClickCnt = 0;
        }
      }
    }
  }
};
#endif // __OPENBUTTON_H__
