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
    Clear();
  }
  virtual ~NIGIRIHit(){}


  virtual void Copy(NIGIRIHit& obj){
        obj.ch=ch;
        obj.finets=finets;
        obj.cshort=cshort;
        obj.clong=clong;
        obj.baseline=baseline;
        obj.nsample=nsample;
        obj.pulse=pulse;
  }
  void Clear(){
        ch = -1;
        finets  = 0;
        cshort = 0;
        clong  = 0;
        baseline = 0;
        nsample = 0;
        pulse.clear();
  }
  void Print(){
      cout<<"ch = "<<ch<<endl;
      cout<<"finets = "<<finets<<endl;
      cout<<"clong = "<<clong<<endl;
      cout<<"pulse 0 = "<<pulse[0]<<endl;
  }
    Short_t ch ;//channel number
    Double_t finets;//finets
    Double_t cshort;//charge short
    Double_t clong;//charge long
    Double_t baseline;//baseline
    Short_t nsample;
    std::vector<UShort_t> pulse;//pulse

  /// \cond CLASSIMP
  ClassDef(NIGIRIHit,1);
  /// \endcond
};

class NIGIRI : public TObject {
public:
  //! default constructor
  NIGIRI(){
      Clear();
  }
  virtual ~NIGIRI(){}
  void Clear(){
      event_size = 0;
      board_id = -1;
      board_fail_flag = 0;
      event_format_reserved = 0;
      pattern = 0;
      channel_mask = 0;

      ts = 0;
      evt = 0;
      b = -1;
      fmult = 0;
      for (size_t idx=0;idx<fhits.size();idx++){
          delete fhits[idx];
      }
      fhits.clear();
  }
  void Print(){
      cout<<"\n******* Event header: *****"<<endl;
      cout<<"event_size = "<<event_size<<endl;
      cout<<"board_id = "<<board_id<<endl;
      cout<<"board_fail_flag = "<<board_fail_flag<<endl;
      cout<<"event_format_reserved = "<<event_format_reserved<<endl;
      cout<<"pattern = "<<pattern<<endl;
      cout<<"channel_mask = "<<channel_mask<<endl;
      cout<<"b = "<<b<<endl;
      cout<<"evt = "<<evt<<endl;
      cout<<"ts = "<<ts<<endl;
      cout<<"fmult = "<<fmult<<endl;
  }
  void DecodeHeaderZsp(int* words,int boardno){
      b = boardno;
      evt = words[2]+1;
      channel_mask = words[1];
      event_size = words[3]/4;
      UInt_t tslsb = (UInt_t) words[5];
      UInt_t tsmsb = (UInt_t) words[4];
      ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;
  }
  void DecodeHeaderRaw(int* words,int ipos, int boardno){
      b = boardno;
      event_size = words[ipos+0]&0xFFFFFFF;
      board_id = (words[ipos+1]&0xF8000000)>>27;
      board_fail_flag = (words[ipos+1]&0x4000000)>>26;
      event_format_reserved = (words[ipos+1]&0x1000000)>>24;
      pattern = (words[ipos+1]&0xFFFF00)>>8;
      int channel_mask_lsb = words[ipos+1]&0xFF;
      int channel_mask_msb = (words[ipos+2]&0xFF000000)>>24;
      channel_mask = (channel_mask_msb<<8)|channel_mask_lsb;
      evt = (words[ipos+2]&0xFFFFFF) +1;
      unsigned int trigger_time_tag = words[ipos+3]&0xFFFFFFFF;
      ts = ((unsigned long long)pattern)<<32|(unsigned long long)trigger_time_tag;
  }

  virtual void Copy(NIGIRI& obj){
      obj.event_size = event_size;
      obj.board_id = board_id;
      obj.board_fail_flag = board_fail_flag;
      obj.event_format_reserved = event_format_reserved;
      obj.event_format_reserved = event_format_reserved;
      obj.pattern = pattern;
      obj.channel_mask = channel_mask;
      obj.evt=evt;
      obj.b=b;
      obj.ts=ts;

      for (vector<NIGIRIHit*>::iterator hitin_it=fhits.begin(); hitin_it!=fhits.end(); hitin_it++){
          NIGIRIHit* clonehit = new NIGIRIHit;
          NIGIRIHit* originhit = *hitin_it;
          originhit->Copy(*clonehit);
          obj.AddHit(clonehit);
      }
      obj.fmult=fmult;
  }
  Int_t GetMult(){return fmult;}
  NIGIRIHit* GetHit(unsigned short n){return fhits.at(n);}
  void AddHit(NIGIRIHit* hit){
    fmult++;
    fhits.push_back(hit);
  }
  Int_t fmult;

  //! common stuff
  Int_t event_size;
  Int_t board_id;
  Int_t board_fail_flag;
  Int_t event_format_reserved;
  Int_t pattern;
  Int_t channel_mask;

  Int_t evt;//evt number
  Short_t b;//board number
  ULong64_t ts;//timestamp
  std::vector<NIGIRIHit*> fhits;
  /// \cond CLASSIMP
  ClassDef(NIGIRI,1);
  /// \endcond
};

class v17xxevent : public TObject {
public:
  //! default constructor
  v17xxevent(){

  }
  virtual ~v17xxevent(){}
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
  ClassDef(v17xxevent,1);
  /// \endcond
};


#endif

