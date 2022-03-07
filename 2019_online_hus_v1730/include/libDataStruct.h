#ifndef __libDataStruct_H
#define __libDataStruct_H

#include "TObject.h"
#include "TROOT.h"
#include <vector>
#include <iostream>
#include <bitset>

#define TDC_N_CHANNEL 64
#define TDC_MAX_MULT 3

using namespace std;
/*!
  Container for the full beam, tof, beta and pid information
*/
class TDCHit : public TObject {
public:
  //! default constructor
  TDCHit(){
      Clear();
  }
  virtual ~TDCHit(){}
  void Clear(){
      ch = -9999;
      t = -9999;
  }
    UShort_t ch ;//channel number
    Int_t t;//baseline
  /// \cond CLASSIMP
  ClassDef(TDCHit,1);
  /// \endcond
};

class NIGIRIHit : public TObject {
public:
  //! default constructor
  NIGIRIHit(){
      evt_type = -1;
      evt = 0;
      b = 0;
      ch = 0;
      ts = 0;
      finets  = 0;
      cshort = 0;
      clong  = 0;
      baseline = 0;
      losttrigger = 0;
      overrange = 0 ;
      tdcmult = 0;
      nsample = 0;
      pulse.clear();
  }
  virtual ~NIGIRIHit(){}

  void Clear(){
        evt_type = -1;
	evt = 0;
        b = -1;
        ch = -1;
	ts = 0;
	finets  = 0;
	cshort = 0;
	clong  = 0;
	baseline = 0;
        losttrigger = 0;
        overrange = 0 ;        
        nsample = 0;
        pulse.clear();
        tdcmult = 0;
        tdc_ch.clear();
        tdc_t.clear();
  }

  void AddTDCHit(UShort_t ch,Int_t t){
      tdc_ch.push_back(ch);
      tdc_t.push_back(t);
      tdcmult++;        
  }

  void Print(){
      cout<<"evt_type = "<<(Int_t)evt_type<<endl;
      cout<<"b = "<<b<<endl;
      cout<<"ch = "<<ch<<endl;
      cout<<"ts = "<<ts<<endl;
      cout<<"finets = "<<finets<<endl;
      cout<<"clong = "<<clong<<endl;
      cout<<"pulse 0 = "<<pulse[0]<<endl;
  }

    Char_t evt_type;
    Char_t overrange;
    Char_t losttrigger;//trigger lost flag (=1: lost)
    Int_t evt;//evt number
    Short_t b;//board number
    Short_t ch ;//channel number
    ULong64_t ts;//timestamp
    Double_t finets;//finets
    Int_t cshort;//charge short
    Int_t clong;//charge long
    Short_t baseline;//baseline
    Short_t nsample;
    std::vector<UShort_t> pulse;//pulse
    Int_t tdcmult;
    std::vector<UShort_t> tdc_ch;
    std::vector<UShort_t> tdc_t;

  /// \cond CLASSIMP
  ClassDef(NIGIRIHit,1);
  /// \endcond
};


class NIGIRI : public TObject {
public:
  //! default constructor
  NIGIRI(){
      fmult = 0;
  }
  virtual ~NIGIRI(){}
  void Clear(){
      fmult = 0;
      for (size_t idx=0;idx<fhits.size();idx++){
          delete fhits[idx];
      }      
      fhits.clear();
  }
  Int_t GetMult(){return fmult;}
  NIGIRIHit* GetHit(unsigned short n){return fhits.at(n);}
  void AddHit(NIGIRIHit* hit){
	fmult++;
	fhits.push_back(hit);
        //cout<<"add!"<<fmult<<endl;
  }
  Int_t fmult;
  std::vector<NIGIRIHit*> fhits;
  /// \cond CLASSIMP
  ClassDef(NIGIRI,1);
  /// \endcond
};

class v1730event : public TObject {
public:
  //! default constructor
  v1730event(){

  }
  virtual ~v1730event(){}
  void Clear(Option_t * = ""){
      event_size = -9999;
      board_id = -9999;
      board_fail_flag = -9999;
      event_format_reserved = -9999;
      pattern = -9999;
      channel_mask = -9999;
      event_counter = 0;
      trigger_time_tag = 0;
      trigger_time_tag_extended = 0;
  }
  void Print(Option_t *option = "") const {
      cout<<"************************"<<endl;
      cout<<"event_size = "<<event_size<<endl;
      cout<<"board_id = "<<board_id<<endl;
      cout<<"board_fail_flag = "<<board_fail_flag<<endl;
      cout<<"event_format_reserved = "<<event_format_reserved<<endl;
      cout<<"pattern = "<<pattern<<endl;
      cout<<"channel_mask = "<<std::bitset<16>(channel_mask)<<endl;
      cout<<"event_counter = "<<event_counter<<endl;
      cout<<"trigger_time_tag = "<<trigger_time_tag<<endl;
      cout<<"trigger_time_tag_extended = "<<trigger_time_tag_extended<<endl;
  }
  void decode(int* words,int ipos){
      event_size = words[ipos+0]&0xFFFFFFF;
      board_id = (words[ipos+1]&0xF8000000)>>27;
      board_fail_flag = (words[ipos+1]&0x4000000)>>26;
      event_format_reserved = (words[ipos+1]&0x1000000)>>24;
      pattern = (words[ipos+1]&0xFFFF00)>>8;
      int channel_mask_lsb = words[ipos+1]&0xFF;
      int channel_mask_msb = (words[ipos+2]&0xFF000000)>>24;
      channel_mask = (channel_mask_msb<<8)|channel_mask_lsb;
      event_counter = words[ipos+2]&0xFFFFFF;
      trigger_time_tag = words[ipos+3]&0xFFFFFFFF;
      trigger_time_tag_extended = ((unsigned long long)pattern)<<32|(unsigned long long)trigger_time_tag;
  }
  int event_size;
  int board_id;
  int board_fail_flag;
  int event_format_reserved;
  int pattern;
  int channel_mask;
  unsigned int event_counter;
  unsigned int trigger_time_tag;
  unsigned long long trigger_time_tag_extended;

  /// \cond CLASSIMP
  ClassDef(v1730event,1);
  /// \endcond
};


#endif

