#ifndef __libDataStruct_H
#define __libDataStruct_H

#include "TObject.h"
#include "TROOT.h"
#include <vector>
#include <iostream>
#include <bitset>

#define TDC_N_CHANNEL 64
#define TDC_MAX_MULT 3


#define V1740_DGTZ_CLK_RES 8
#define V1730_DGTZ_CLK_RES 2
#define LUPO_CLK_RES 10

using namespace std;
/*!
  Container for the full beam, tof, beta and pid information
*/

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <string.h>

class BrikenTreeData  {
public:
   BrikenTreeData() { }
   ~BrikenTreeData() {}
   double E;
   uint64_t T;
   uint16_t Id;
   uint16_t type;
   uint16_t Index1;
   uint16_t Index2;
   uint16_t InfoFlag;
   std::string Name;
   void Clear(){
       E = -10;
       T = 0;
       Id = 9999;
       type = 9999;
       Index1 = 9999;
       Index2 = 9999;
       InfoFlag = 9999;
       Name = "";
   };
};


class NIGIRIHit : public TObject {
public:
    //! default constructor
    NIGIRIHit(){
        Clear();
    }
    virtual ~NIGIRIHit(){Clear();}

    virtual void Copy(NIGIRIHit& obj){
        obj.ts_corr_type = ts_corr_type;
        obj.ts = ts;
        obj.sampling_interval = sampling_interval;
        obj.trig_pos = trig_pos;
        obj.ch=ch;
        obj.finets=finets;
        obj.cshort=cshort;
        obj.clong=clong;
        obj.baseline=baseline;
        obj.nsample=nsample;
        obj.pulse=pulse;
        obj.pulse_ap1=pulse_ap1;
        obj.pulse_ap2=pulse_ap1;
    }
    void Clear(){
        ts_corr_type = 0;
        ts = 0;
        sampling_interval = 0;
        trig_pos = 0;
        ch = -1;
        finets  = 0;
        cshort = 0;
        clong  = 0;
        baseline = 0;
        nsample = 0;
        pulse.clear();
        pulse_ap1.clear();
        pulse_ap2.clear();
    }
    void ClearHeaders(){
        ts_corr_type = 0;
        ts = 0;
        sampling_interval = 0;
        trig_pos = 0;
        ch = -1;
        finets  = 0;
        cshort = 0;
        clong  = 0;
        baseline = 0;
        nsample = 0;
    }
    void Print(){
        cout<<"ch = "<<ch<<endl;
        cout<<"ch ts = "<<ts<<endl;
        cout<<"sampling_interval = "<<sampling_interval<<endl;
        cout<<"trigger position = "<<trig_pos<<endl;
        cout<<"finets = "<<finets<<endl;
        cout<<"clong = "<<clong<<endl;
        cout<<"pulse size = "<<pulse.size()<<endl;
        cout<<"pulse ap1 size = "<<pulse_ap1.size()<<endl;
        cout<<"pulse ap1 size = "<<pulse_ap2.size()<<endl;
    }
    UChar_t ts_corr_type;
    ULong64_t ts;
    UShort_t sampling_interval;
    UShort_t trig_pos;
    Short_t ch ;//channel number
    Double_t finets;//finets
    Double_t cshort;//charge short / energy 1
    Double_t clong;//charge long / energy 2
    Double_t baseline;//baseline
    Short_t nsample;
    std::vector<UShort_t> pulse;//pulse
    std::vector<UShort_t> pulse_ap1;//analogprobe1
    std::vector<UShort_t> pulse_ap2;//analogprobe2
    std::vector<UShort_t> pulse_dp1;//analogprobe1
    std::vector<UShort_t> pulse_dp2;//analogprobe2
    void processPulseV1740(ULong64_t boardts, Int_t nsbl, UShort_t ledthr,UShort_t trig_pos_in, UShort_t sampling_interval_in){
        ts = boardts;
        trig_pos = trig_pos_in;
        sampling_interval = sampling_interval_in;
        baseline = 0;
        clong  = 0;
        finets = -1;
        Int_t ii=0;
        for (std::vector<UShort_t>::iterator it = pulse.begin() ; it != pulse.end(); ++it){
            double currpulse =(double) *it;
            if (ii<nsbl)
                baseline+=currpulse;
            if (currpulse>clong) {
                clong = currpulse;
                cshort = ii;//position of maximum
            }
            if (currpulse>ledthr&&finets==-1){
                finets = ii;
            }
            ii++;
        }
        baseline = baseline /(Double_t)nsbl;
        //! calculate precise timing of channels
        if (finets>=0){
            ts = ts - trig_pos*sampling_interval+(ULong64_t)finets*sampling_interval;
        }

        //! special function to handle pulse ahead of trigger position
//        if (finets==0){//pulse with trigger
//            baseline = pulse.back();//last element as baseline
//        }
//        //! special function to reject pulse comes very late in time
//        if (cshort == pulse.size()-1&&pulse.size()>nsbl){
//            clong  = 0;
//        }
        //! only  pulse above threshold give energy>0
        if (finets>=0) clong  = clong  - baseline;
        else clong  = -9999;//no data above threshold
    };
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
    virtual ~NIGIRI(){Clear();}
    void Clear(){
        event_size = 0;
        board_id = -1;
        board_fail_flag = 0;
        event_format_reserved = 0;
        pattern = 0;
        channel_mask = 0;
        trig_ch = -1;

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
        cout<<"******* Event header: *****"<<endl;
        cout<<"event_size = "<<event_size<<endl;
        cout<<"board_id = "<<board_id<<endl;
        cout<<"board_fail_flag = "<<board_fail_flag<<endl;
        cout<<"event_format_reserved = "<<event_format_reserved<<endl;
        cout<<"trig_ch = "<<trig_ch<<endl;
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
        ts = ts*V1740_DGTZ_CLK_RES;
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
        ts = ts*V1740_DGTZ_CLK_RES;
    }
    void MergePulse(NIGIRI* data,ULong64_t boardts, Int_t nsbl, UShort_t* ledthr,UShort_t trig_pos_in, UShort_t sampling_interval_in){
        for (Int_t i=0;i<fmult;i++){
            GetHit(i)->ClearHeaders();
            GetHit(i)->nsample = GetHit(i)->pulse.size()+data->GetHit(i)->pulse.size();
            GetHit(i)->pulse.insert(std::end(GetHit(i)->pulse),std::begin(data->GetHit(i)->pulse),std::end(data->GetHit(i)->pulse));
            GetHit(i)->processPulseV1740(boardts,nsbl,ledthr[i],trig_pos_in,sampling_interval_in);
        }
    }

    virtual void Copy(NIGIRI& obj){
        obj.event_size = event_size;
        obj.board_id = board_id;
        obj.board_fail_flag = board_fail_flag;
        obj.event_format_reserved = event_format_reserved;
        obj.trig_ch = trig_ch;
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
    virtual void CopyHeader(NIGIRI& obj){
        obj.event_size = event_size;
        obj.board_id = board_id;
        obj.board_fail_flag = board_fail_flag;
        obj.event_format_reserved = event_format_reserved;
        obj.trig_ch = trig_ch;
        obj.pattern = pattern;
        obj.channel_mask = channel_mask;
        obj.evt=evt;
        obj.b=b;
        obj.ts=ts;
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
    Short_t trig_ch;

    Int_t evt;//evt number
    Short_t b;//board number
    ULong64_t ts;//timestamp
    std::vector<NIGIRIHit*> fhits;
    /// \cond CLASSIMP
    ClassDef(NIGIRI,1);
    /// \endcond
};


#endif

