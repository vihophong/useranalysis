
/* ************************ DO NOT REMOVE  ****************************** */
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "pdecoder.h"

#include <dpp.h>
#include <libDataStruct.h>
#include <bitset>
#include <stdint.h>
#include <vector>
#include <map>

#include <TSystem.h>
#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TCanvas.h>

#include <iostream>
#include <fstream>

#include <MyMainFrame.h>
//#include <DataStruct_dict.C>

//#define SLOW_ONLINE 100000

#define V1730_TRIGGER_CLOCK_RESO 8
#define V1730_EVENT_TYPE 1
#define V1730_N_CH 16
#define V1730_MAX_N_CH 16
#define V1730_PACKET 52
#define NSBL 8

#define N_REFESH_WF 1000
#define N_MAX_WF_LENGTH 1000

using namespace std;

//! histograms and trees
TH2F *ht2d;
TH2F *he2d;

TH1F *hwf1d[V1730_N_CH];
TH2F *hwf2d[V1730_N_CH];
TH2F *hpsd2d[V1730_N_CH];

TH1F *hrate;
TH1F *hratetmp;
TH1F *hratetmpscale;

TH1F *h_ts[V1730_N_CH];

TH1F *ht0_t2;

MyMainFrame* mf;

Int_t ch_thr[V1730_MAX_N_CH];
Int_t ch_ecal0[V1730_MAX_N_CH];
Int_t ch_ecal1[V1730_MAX_N_CH];

ULong64_t ts_begin[V1730_MAX_N_CH];

ULong64_t ts_begin2[V1730_MAX_N_CH];

void Init(){    
    std::ifstream inpf("channel_calib.txt");
    if (inpf.fail()){
        cout<<"No Configuration table is given"<<endl;
        return;
    }

    cout<<"Start reading calibration table channel_calib.txt"<<endl;
    Int_t channel;
    Double_t threshold,ecal0,ecal1;
    Int_t mm=0;

    while (inpf.good()){
        inpf>>channel>>threshold>>ecal0>>ecal1;
        cout<<channel<<"-"<<threshold<<"-"<<ecal0<<"-"<<ecal1<<endl;
        ch_ecal0[channel]=ecal0;
        ch_ecal1[channel]=ecal1;
        ch_thr[channel]=threshold;
        mm++;
    }

    ht2d = new TH2F("t2d","Time Spectra 2D",V1730_N_CH,0,V1730_N_CH,550,0,550);
    he2d = new TH2F("e2d","Energy Spectra 2D",V1730_N_CH,0,V1730_N_CH,2000,0,40000);    
    for (int i=0;i<V1730_N_CH;i++){
        hwf1d[i] = new TH1F (Form("wf1730_1d_%d",i),Form("Waveform 1d v1730 %d",i), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH );
        hwf2d[i] = new TH2F (Form("wf1730_2d_%d",i),Form("Waveform 2d v1730 %d",i), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH, 1700, 0,17000 );
        hpsd2d[i] = new TH2F(Form("psd_2d_%d",i),Form("PSD 2d v1730 %d",i),500,0,40000,500,-3,3);
        h_ts[i] = new TH1F (Form("hts1730_1d_%d",i),Form("HTS v1730 %d",i),3600,0,3600);
        ts_begin[i]=0;
        ts_begin2[i]=0;
    }
    hrate=new TH1F("hrate","hrate",V1730_N_CH,0,V1730_N_CH);
    hratetmp=new TH1F("hratetmp","hratetmp",V1730_N_CH,0,V1730_N_CH);
    hratetmpscale=new TH1F("hratetmpscale","hratetmpscale",V1730_N_CH,0,V1730_N_CH);

    ht0_t2 = new TH1F("ht0_t2","Time correlation T2-T0 (ns)",500,-50,50);

    mf=new MyMainFrame(gClient->GetRoot(),200,200);
    mf->GetConfig();
    mf->InitHistograms();
    mf->DrawFrameCanvas();

}

void ProcessSingleEvent(NIGIRIHit* data){
    if (data->evt_type == V1730_EVENT_TYPE){
        int ch  = data->ch;
        unsigned long long ts = data->ts;
        Double_t ts_second = ((Double_t)ts)/1e9*V1730_TRIGGER_CLOCK_RESO;
        h_ts[ch]->Fill(ts_second);
        //! stuff for rate calculation
        if (data->clong>ch_thr[ch]){
            if (mf->GetFlagRateUpdate(ch)){
                ts_begin[ch]=ts;
            }
            mf->ResetFlagRateUpdate(ch);
            Double_t ts_s=((Double_t)((Long64_t)ts-(Long64_t)ts_begin[ch]))/1e9*V1730_TRIGGER_CLOCK_RESO;

            hratetmp->Fill(ch);
            hratetmpscale->SetBinContent(ch+1,ts_s);
        }
        //!----------------------------

        int itcnt=0;
        if (data->clong>ch_thr[ch]){
            double ecal=data->clong*ch_ecal1[ch]+ch_ecal0[ch];
            he2d->Fill(ch,ecal);
            ht2d->Fill(ch,data->finets);
            hpsd2d[ch]->Fill(ecal,(Double_t)(data->clong-data->cshort)/(Double_t)data->clong);
        }        
        for (std::vector<UShort_t>::iterator it = data->pulse.begin() ; it != data->pulse.end(); ++it){
            if (itcnt<N_MAX_WF_LENGTH){
                hwf2d[ch]->Fill(itcnt,*it);
                hwf1d[ch]->SetBinContent(itcnt+1,*it);
            }
            itcnt++;
        }
    }else{
        cout<<"ERROR! @.@"<<endl;
    }
}

void CloseMe(){

}

//!------------**********************
//! //!------------**********************
//! //!------------**********************
//! //!------------**********************

//! DPP parameters
int CFD_delay=1;//in Tclk unit, 1 Tclk=2ns
double CFD_fraction=0.5;
double LED_threshold=100;
int gateOffset=10;
int shortGate=20;
int longGate=150;
int nBaseline=16;
int minVarBaseline=100; //criteria for baseline determination
int mode_selection=2;

//! dont care about the following code
uint16_t seperateint(int inint,bool islsb){
  if (islsb) return (uint16_t) (inint&0xFFFF);
  else return (uint16_t) (inint>>16);
}

NIGIRIHit* data;
int init_done = 0;
int n_v1730;
v1730event v1730evt;

int pinit()
{
  if (init_done) return 1;
  init_done = 1;
  gROOT->ProcessLine(".L libDataStruct.so");
  data = new NIGIRIHit;
  Init();

  n_v1730=0;
  return 0;
}

int process_event (Event * e)
{
    //! v1740 packet
    Packet *p1730=e->getPacket(V1730_PACKET);
    if (p1730)
    {
#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
        n_v1730++;
        int* tmp;
        int* words;
        words=(int*) p1730->getIntArray(tmp);
        int nevt=p1730->getPadding();
//        cout<<"\nProcessing events block"<<n_v1730<<" Number of Events="<<nevt<<endl;
        int ipos=0;
        for (int evt=0;evt<nevt;evt++){
            v1730evt.Clear();
            v1730evt.decode(words,ipos);
//            v1730evt.Print();
            ipos+=4;
            //calculate sample length first
            int nch=0;
            for (int ch=0;ch<V1730_MAX_N_CH;ch++){
                if (!(v1730evt.channel_mask & (1 << ch))) continue;
                nch++;
            }
            int nsamples_per_ch = (v1730evt.event_size-4)*2/nch;
//            cout<<"nsamples_per_ch = "<<nsamples_per_ch<<endl;
            //loop all channel with given sample length
            double t_ch0=0;
            double t_ch2=0;
            for (int ch=0;ch<V1730_MAX_N_CH;ch++){
                if (!(v1730evt.channel_mask & (1 << ch))) continue;
                data->Clear();
                data->evt_type = V1730_EVENT_TYPE;

                data->b = 0;//for sorter
                data->evt = v1730evt.event_counter;
                data->ts = v1730evt.trigger_time_tag_extended;
                data->ch = ch;//for sorter                
                data->nsample = nsamples_per_ch;

                int isample=0;
                UShort_t WaveLine[data->nsample];
                for (int i=0;i<nsamples_per_ch/2;i++){
                    unsigned short sample0=words[ipos]&0x7FFF;
                    unsigned short sample1=(words[ipos]>>16)&0x7FFF;
//                    cout<<"sample "<<isample<<" = "<<sample0<<endl;
                    WaveLine[isample]=sample0;
                    isample++;
//                    cout<<"sample "<<isample<<" = "<<sample1<<endl;
                    WaveLine[isample]=sample1;
                    isample++;
                    data->pulse.push_back(sample0);//lsb                    
                    data->pulse.push_back(sample1);//msb
                    ipos++;
                }
                //analyze data
                if (data->nsample>NSBL){
                    dpp *oj=new dpp(data->nsample,WaveLine);
                    oj->baselineMean(nBaseline,minVarBaseline);
                    double timeData = 0;
                    int cShort,cLong;
                    if (mode_selection==0) {
                        timeData=oj->led(ch_thr[ch]);
                        cout<<"ch"<<ch<<"- tsdata"<<timeData<<endl;
                    }else if (mode_selection==1) {
                        timeData=oj->ledWithCorr(ch_thr[ch]);
                    }else if (mode_selection==2){
                        oj->makeCFD(ch_thr[ch],CFD_delay,CFD_fraction);
                        timeData=oj->cfdFast();
                    }else if (mode_selection==3){
                        oj->makeCFD(ch_thr[ch],CFD_delay,CFD_fraction);
                        timeData=oj->cfdSlow();
                    }

                    oj->chargeInter(cShort,cLong,timeData,gateOffset,shortGate,longGate);
                    data->cshort = cShort;
                    data->clong = cLong;                    
                    //data->cshort = 0;
                    //data->clong = oj->maxAdcPos(N_MAX_WF_LENGTH)-oj->bL;
                    //data->clong = oj->bL-oj->minAdcNeg(N_MAX_WF_LENGTH);
                    data->baseline = oj->bL;
                    data->finets = timeData;

                    if (ch==0) {t_ch0=timeData;}//cout<<"t_ch0 = "<<t_ch0<<endl;}
		    
                    if (ch==2&&timeData>0) {t_ch2=timeData;}//cout<<"t_ch2 = "<<t_ch2<<endl;}

                    delete oj;
                }//analyze data                
                ProcessSingleEvent(data);
            }//end of channel loop
            if (t_ch0>40&&t_ch2>40&&t_ch0<250&&t_ch2<250) ht0_t2->Fill((t_ch2-t_ch0)*2);

        }//end of event loop
        delete p1730;
    }//end of packet loop
    return 0;
}

int pclose(){
    CloseMe();
    cout<<"Finish!"<<endl;
    return 0;
}

