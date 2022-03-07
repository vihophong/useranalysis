
/* ************************ DO NOT REMOVE  ****************************** */
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "myanalysis.h"
#include <time.h>

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
#include <TRandom.h>
#include <TString.h>
#include <TCanvas.h>

#include <iostream>
#include <fstream>

#include <correlation.h>
#include "TThread.h"

#define FIX_EVENT_SIZE_V1730RAW 1754

//#define SLOW_ONLINE 100000
#define RATE_CAL_REFESH_SECONDS 10

#define MAX_N_BOARD 20
#define MAX_N_V1730_BOARD 1
#define V1730_MAX_N_CH 16
#define V1740_N_MAX_CH 64
//! parameters for V1740
#define NSBL 8
#define N_MAX_WF_LENGTH 90
UShort_t trig_pos = N_MAX_WF_LENGTH*30/100;//unit of sample
UShort_t sampling_interval = 16*16;//unit of ns

TFile* file0 = 0;
TTree* tree = 0;
NIGIRI* treedata[3];

NIGIRI* data;

#define DIGITAL_PROBE_OFFSET 0
#define DIGITAL_PROBE_GAIN 500

#define TRG_PROBE_OFFSET 0
#define TRG_PROBE_GAIN 500


TH2F *hap1trace2d[V1730_MAX_N_CH];
TH2F *hap2trace2d[V1730_MAX_N_CH];

TH2F *he2d;
TH1F *he1d[V1730_MAX_N_CH];
TH1F *hap1trace1d[V1730_MAX_N_CH];
TH1F *hap2trace1d[V1730_MAX_N_CH];
TH1F *hdptrace1d[V1730_MAX_N_CH];
TH1F *htrgtrace1d[V1730_MAX_N_CH];

TH2F *hrate2d;
TH1F *hrate1d[V1730_MAX_N_CH];
ULong64_t ts_begin;

int nevts = 0;
TCanvas * c1;

#define MAX_MAP1_LENGTH 50
#define MAX_MAP2_LENGTH 100
#define TS_WIN_LOW 300
#define TS_WIN_HIGH 300
std::multimap <ULong64_t,NIGIRI*> datamap_hit1; //! sort by timestamp
std::multimap <ULong64_t,NIGIRI*> datamap_hit2; //! sort by timestamp
std::multimap<ULong64_t,NIGIRI*>::iterator it_datamap_hit1;
std::multimap<ULong64_t,NIGIRI*>::iterator it_datamap_hit2;

Double_t tof_offset = 0;
TH1F* hcorr[6];
TH2F* hEF11LvsEF11R;
TH2F* hEVetovsEF11;
TH2F* hEF11vsTOF;
TH2F* hEVetovsTOF;

TH1F* he1dPHA[V1730_MAX_N_CH*2];

#define PULSE_SAMPLE_LENGTH 496
UShort_t pulse_samples[PULSE_SAMPLE_LENGTH];//analogprobe2

void Init(){
    for (Int_t ch=0;ch<V1730_MAX_N_CH;ch++){
        htrgtrace1d[ch]=new TH1F(Form("htrgtrace1d%d",ch),Form("htrgtrace1d%d",ch),500,0,500);
        hdptrace1d[ch]=new TH1F(Form("hdptrace1d%d",ch),Form("hdptrace1d%d",ch),500,0,500);
        hap1trace1d[ch]=new TH1F(Form("hap1trace1d%d",ch),Form("hap1trace1d%d",ch),500,0,500);
        hap2trace1d[ch]=new TH1F(Form("hap2trace1d%d",ch),Form("hap2trace1d%d",ch),500,0,500);
        hap1trace2d[ch]=new TH2F(Form("hap1trace2d%d",ch),Form("hap1trace2d%d",ch),500,0,500,1000,0,pow(2,14)+1000);
        hap2trace2d[ch]=new TH2F(Form("hap2trace2d%d",ch),Form("hap2trace2d%d",ch),500,0,500,1000,0,pow(2,14)+1000);
        he1d[ch]=new TH1F(Form("he1d%d",ch),Form("he1d%d",ch),1000,0,20000);
        hrate1d[ch]=new TH1F(Form("hrate1d%d",ch),Form("hrate1d%d",ch),3600,0,3600);
    }
    for (Int_t ch=0;ch<V1730_MAX_N_CH*2;ch++){
        he1dPHA[ch] = new TH1F(Form("he1dPHA%d",ch),Form("he1dPHA%d",ch),2000,0,200000);
    }

    c1=new TCanvas("c1","c1",900,700);
    he2d = new TH2F("he2d","he2d",V1730_MAX_N_CH,0,V1730_MAX_N_CH,1000,0,20000);
    hrate2d = new TH2F("hrate2d","hrate2d",3600,0,3600,V1730_MAX_N_CH,0,V1730_MAX_N_CH);
    he2d->Draw("colz");
    ts_begin = 0;


    //! reco trees
    for (Int_t i=0;i<6;i++) hcorr[i] = new TH1F(Form("hcorr%d",i),Form("hcorr%d",i),5000,-1,1);

    for (Int_t i=0;i<PULSE_SAMPLE_LENGTH;i++){
        pulse_samples[i]=i;
    }

    for (Int_t i=0;i<3;i++){
        treedata[i] = new NIGIRI;
    }

//    tree = new TTree("tree","tree");
//    tree->Branch("hit1",&treedata[0]);
//    tree->Branch("hit2",&treedata[1]);
//    tree->Branch("hit3",&treedata[2]);

//    hEF11LvsEF11R = new TH2F("hEF11LvsEF11R","hEF11LvsEF11R",500,0,30000,500,0,30000);
//    hEVetovsEF11 = new TH2F("hEVetovsEF11","hEVetovsEF11",500,0,30000,500,0,30000);
//    hEF11vsTOF = new TH2F("hEF11vsTOF","hEF11vsTOF",500,0,200,500,0,30000);
//    hEVetovsTOF = new TH2F("hEVetovsTOF","hEVetovsTOF",500,0,200,500,0,30000);

    pupdate(c1,2);
}

void ProcessEvent(NIGIRI* data_now){

    //! PHA
    if (data_now->b==12||data_now->b==13){
        NIGIRIHit* hit = data_now->GetHit(0);
        Int_t ch = hit->ch+(data_now->b-12)*V1730_MAX_N_CH;
        he1dPHA[ch]->Fill(hit->clong);
    }

    //! PSD
    if (data_now->b<=1){
        //data_now->Print();
        NIGIRIHit* hit = data_now->GetHit(0);
        Int_t ch = hit->ch;
        //! calibratiom
        if (ch==0){
            hit->clong = hit->clong;//*0.207834689829088-282.530008903132;
        }else if (ch==1){
            hit->clong = hit->clong;//*0.208949085043504-286.484743337769;
        }
//        if (ch==1)
//            cout<<ch<<"\t"<<data_now->ts<<endl;
//        cout<<data_now->ts<<endl;
        he1d[ch]->Fill(hit->clong);
        he2d->Fill(ch,hit->clong);
        hap1trace1d[ch]->Reset();
        hap2trace1d[ch]->Reset();
        hdptrace1d[ch]->Reset();
        htrgtrace1d[ch]->Reset();
        //! for single trace
        int cnt = 0;
        for (std::vector<UShort_t>::iterator it =hit->pulse_ap1.begin() ; it != hit->pulse_ap1.end(); ++it){
            UShort_t adcitem = *it;
            hap1trace1d[ch]->SetBinContent(cnt,adcitem);
            hap1trace2d[ch]->Fill(cnt,adcitem);
            cnt++;
        }
        cnt = 0;
        for (std::vector<UShort_t>::iterator it =hit->pulse_dp1.begin() ; it != hit->pulse_dp1.end(); ++it){
            UShort_t adcitem = *it;
            hdptrace1d[hit->ch]->SetBinContent(cnt,adcitem*DIGITAL_PROBE_GAIN+DIGITAL_PROBE_OFFSET);
            cnt++;
        }
        cnt = 0;
        for (std::vector<UShort_t>::iterator it =hit->pulse_dp2.begin() ; it != hit->pulse_dp2.end(); ++it){
            UShort_t adcitem = *it;
            htrgtrace1d[hit->ch]->SetBinContent(cnt,adcitem*TRG_PROBE_GAIN+TRG_PROBE_OFFSET);
            cnt++;
        }
        //! Rate
        if (ts_begin==0){
            ts_begin = data_now->ts;
        }
        if (ts_begin>0){
            hrate1d[ch]->Fill(((Double_t)data_now->ts-(Double_t)ts_begin)/1e9);
            hrate2d->Fill(ch,((Double_t)data_now->ts-(Double_t)ts_begin)/1e9);
        }

        //! Correlation
        if (datamap_hit1.size()>MAX_MAP1_LENGTH){
            for(it_datamap_hit1=datamap_hit1.begin();it_datamap_hit1!=datamap_hit1.end();it_datamap_hit1++){
                Long64_t ts=(Long64_t)it_datamap_hit1->first;
                NIGIRI* hit1=(NIGIRI*)it_datamap_hit1->second;
                Long64_t ts1 = ts - TS_WIN_LOW;
                Long64_t ts2 = ts + TS_WIN_HIGH;
                Long64_t corrts = 0;
                Int_t ncorr = 0;
                it_datamap_hit2 = datamap_hit2.lower_bound(ts1);
                Int_t ncorr_ch[V1730_MAX_N_CH];
                memset(ncorr_ch,0,sizeof(ncorr_ch));
                while(it_datamap_hit2!=datamap_hit2.end()&&it_datamap_hit2->first<ts2){
                    corrts = (Long64_t) it_datamap_hit2->first;
                    NIGIRI* hit2=(NIGIRI*)it_datamap_hit2->second;
                    //! Fill Correlation here
//                    if (hit2->GetHit(0)->ch==1){
//                        hEF11LvsEF11R->Fill(hit2->GetHit(0)->clong,hit1->GetHit(0)->clong);
//                    }
//                    if(hit2->GetHit(0)->ch==2)
//                        hEVetovsEF11->Fill(hit1->GetHit(0)->clong,hit2->GetHit(0)->clong);
//                    if(hit2->GetHit(0)->ch==2)
//                        hEF11vsTOF->Fill(hit2->GetHit(0)->finets-hit1->GetHit(0)->finets,hit1->GetHit(0)->clong);
//                    if(hit2->GetHit(0)->ch==2)
//                        hEVetovsTOF->Fill(hit2->GetHit(0)->finets-hit1->GetHit(0)->finets,hit2->GetHit(0)->clong);
                    hcorr[hit2->GetHit(0)->ch-1]->Fill(hit2->GetHit(0)->finets-hit1->GetHit(0)->finets);
//                    cout<<hit1->GetHit(0)->finets<<"\t"<<hit2->GetHit(0)->finets<<endl;
                    ncorr_ch[hit2->GetHit(0)->ch]++;
                    if (hit2->GetHit(0)->ch==1 && ncorr_ch[hit2->GetHit(0)->ch]==1){
                        treedata[1]->Clear();
                        hit2->Copy(*treedata[1]);
                        std::vector<UShort_t> vec(pulse_samples, pulse_samples + PULSE_SAMPLE_LENGTH);
                        treedata[1]->GetHit(0)->pulse_ap2  = vec;
                    }
                    if (hit2->GetHit(0)->ch==2 &&  ncorr_ch[hit2->GetHit(0)->ch]==1){
                        treedata[2]->Clear();
                        hit2->Copy(*treedata[2]);
                        std::vector<UShort_t> vec(pulse_samples, pulse_samples + PULSE_SAMPLE_LENGTH);
                        treedata[2]->GetHit(0)->pulse_ap2 = vec;
                    }
                    ncorr++;
                    it_datamap_hit2++;
                }
//                if (ncorr_ch[1]==1&&ncorr_ch[2]==1){
                if (ncorr_ch[1]==1){
                    treedata[0]->Clear();
                    hit1->Copy(*treedata[0]);
                    std::vector<UShort_t> vec(pulse_samples, pulse_samples + PULSE_SAMPLE_LENGTH);
                    treedata[0]->GetHit(0)->pulse_ap2 = vec;
                    if (tree!=0) tree->Fill();
                }

                //! delete all matchhes for map2
//                if (ncorr>0){
//                    for (std::multimap<ULong64_t,NIGIRI*>::iterator it_datamaptmp=datamap_hit2.begin();it_datamaptmp!=it_datamap_hit2;it_datamaptmp++){
//                        NIGIRI* hittmp=it_datamaptmp->second;
//                        hittmp->Clear();
//                        delete hittmp;
//                    }
//                    datamap_hit2.erase(datamap_hit2.begin(),it_datamap_hit2);
//                }

                //! delete map 1 and break (meaning take only earliest ts)
                hit1->Clear();
                delete hit1;
                datamap_hit1.erase(datamap_hit1.begin(),++it_datamap_hit1);
                break;
            }
        }
        //! Keeping fix size of map2
        if (datamap_hit2.size()>MAX_MAP2_LENGTH){
            for(it_datamap_hit2=datamap_hit2.begin();it_datamap_hit2!=datamap_hit2.end();it_datamap_hit2++){
                NIGIRI* hit2=(NIGIRI*)it_datamap_hit2->second;
                hit2->Clear();
                delete hit2;
                datamap_hit2.erase(datamap_hit2.begin(),++it_datamap_hit2);
                break;
            }
        }

        //! Fill data
        if (ch==0){
            NIGIRI* datac=new NIGIRI;
            data_now->Copy(*datac);
            datamap_hit1.insert(make_pair(data_now->ts,datac));
        }else{
            if (ch<3){
                NIGIRI* datac=new NIGIRI;
                data_now->Copy(*datac);
                datamap_hit2.insert(make_pair(data_now->ts,datac));
            }
        }
    }

}

void DoUpdate(){
    //pstatus();
}

void OpenFile(const char* filename){
//    file0 = new TFile(filename,"recreate");
//    tree = new TTree("tree","tree");
    tree = new TTree("tree","tree");
    for (Int_t i=0;i<3;i++){
        treedata[i] = new NIGIRI;
    }
    tree->Branch("hit1",&treedata[0]);
    tree->Branch("hit2",&treedata[1]);
    tree->Branch("hit3",&treedata[2]);
}

void CloseMe(){
//    if (file0) {
//        if (tree) tree->Write();
//        file0->Close();
//    }
//    cout<<nevts<<endl;
    if (tree!=0){
        tree->Reset();
        tree = 0;
    }
}

//!**************************************************
//! Data decoding
//! *************************************************

//! parameters for decoding V1740 zerosuppression
#define V1740_HDR 6

//! packet map
typedef enum{
    NONE = 0,
    LUPO = 1,
    V1740ZSP = 2,
    V1740RAW = 3,
    V1730DPPPHA = 4,
    V1730DPPPSD = 5,
}pmap_decode;

//! full map
//#define N_PACKETMAP 14
//const int packetmap[]={49,50,51,52,53,54,55,56,57,58,59,60,61,100};
//const pmap_decode packetdecode[]={LUPO,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1740ZSP,V1730DPPPHA};

#define N_PACKETMAP 4
const int packetmap[]={50,51,100,101};
const pmap_decode packetdecode[]={V1730DPPPSD,NONE,V1730DPPPHA,V1730DPPPHA};

UShort_t ledthr[MAX_N_BOARD][V1740_N_MAX_CH];
NIGIRI* data_prev[MAX_N_BOARD];

int init_done = 0;


struct refesh_thread_argument
{
  unsigned int refreshinterval;
};
static TThread *refesh_thread = 0;

void rateupdate(void * ptr){
    refesh_thread_argument* ta = (refesh_thread_argument *) ptr;
    unsigned int my_refreshinterval = ta->refreshinterval;
    time_t last_time = time(0);  //force an update on start
    //cout<<"refesh thread initialized, current time since January 1, 1970"<<last_time<<endl;
    while (1)
    {
        time_t x = time(0);
        if ( x - last_time > my_refreshinterval)
      {
        last_time = x;
        DoUpdate();
       }
    }
}

int pinit()
{

  if (init_done) return 1;
  init_done = 1;
#ifdef RATE_CAL_REFESH_SECONDS
  refesh_thread_argument* ta = new refesh_thread_argument;
  ta->refreshinterval = RATE_CAL_REFESH_SECONDS;
  refesh_thread = new TThread(rateupdate,ta);
  refesh_thread->Run();
#endif


  gROOT->ProcessLine(".L libDataStruct.so");
  data=new NIGIRI;
  for (Int_t i=0;i<MAX_N_BOARD;i++){
      data_prev[i]=new NIGIRI;
      for (Int_t j=0;j<V1740_N_MAX_CH;j++){
          ledthr[i][j]=750;
      }
  }

  Init();
  return 0;
}

void decodeV1740raw(Packet* p1740raw){
    //!**************************************************
    //! v1740Raw data packet
    //! *************************************************
    int* tmp;
    int* words;
    words=(int*) p1740raw->getIntArray(tmp);
    int nevt=p1740raw->getPadding();
    int ipos=0;
    for (int evt=0;evt<nevt;evt++){
        //NIGIRI* data=new NIGIRI;
        data->Clear();
        data->DecodeHeaderRaw(words,ipos,p1740raw->getHitFormat());
        ipos+=4;
        //cout<<data->event_size<<endl;
        if (FIX_EVENT_SIZE_V1730RAW > 0){
            data->event_size=FIX_EVENT_SIZE_V1730RAW;
        }
        data->DecodeSamplesRaw(words,ipos);
        ipos+=data->event_size-4;
        ProcessEvent(data);
    }//end of event loop
}

#define TSCORR 6
double thr_abs = 150;

void decodeV1740zsp(Packet* p1740zsp){
    //!**************************************************
    //! v1740 with zerosuppresion data packet
    //! *************************************************
    int* temp;
    int* gg;
    gg=(int*) p1740zsp->getIntArray(temp);
    //! header
    //NIGIRI* data=new NIGIRI;
    int nEvents = p1740zsp->getPadding();
    //cout<<"\n********** "<<nEvents<<" ***********\n"<<endl;
    int k=0;

    for (Int_t i=0;i<nEvents;i++){
        data->Clear();
        int headaddr = k;
        data->DecodeHeaderZsp(&gg[k],p1740zsp->getHitFormat());
        k+=V1740_HDR+V1740_N_MAX_CH;
        //! get number of channels from channel mask
        double min_finets = 99999;
        int ich_min_finets = -1;
        for (int i=0;i<V1740_N_MAX_CH;i++){
            int chgrp = i/8;
            if (((data->channel_mask>>chgrp)&0x1)==0) continue;
            //! header
            NIGIRIHit* chdata=new NIGIRIHit;
            chdata->ch = i;//for sorter
            int nsample = gg[headaddr+V1740_HDR+i];


            if (nsample>NSBL&&nsample<N_MAX_WF_LENGTH){
                //cout<<i<<"bbb"<<nsample<<endl;
                data->board_fail_flag = 1;
            }
            chdata->nsample = nsample;
            UShort_t WaveLine[nsample];
            int ispl = 0;
            for (int j=0;j<nsample/2+nsample%2;j++){
                if (ispl<nsample) {
                    WaveLine[ispl]=gg[k]&0xFFFF;
                    chdata->pulse.push_back(gg[k]&0xFFFF);

                }
                ispl++;
                if (ispl<nsample) {
                    WaveLine[ispl]=(gg[k]>>16)&0xFFFF;
                    chdata->pulse.push_back((gg[k]>>16)&0xFFFF);
                }
                ispl++;
                k++;
            }
            //!--------------------
            if (nsample>NSBL){
                chdata->processPulseV1740(data->ts,NSBL,ledthr[data->b][chdata->ch],trig_pos,sampling_interval);
            }

            if (chdata->finets<min_finets&&chdata->finets>=0){
                min_finets =chdata->finets;
                ich_min_finets = i;
            }
            data->AddHit(chdata);
        }//loop all channels
        data->trig_ch = ich_min_finets;

        if (data->board_fail_flag==1){
            data_prev[data->b]->MergePulse(data,data_prev[data->b]->ts,NSBL,ledthr[data->b],trig_pos,sampling_interval,N_MAX_WF_LENGTH);
        }
        //ProcessEvent(data);
        //! process data
        if (data_prev[data->b]->b>=0){
            if (data_prev[data->b]->board_fail_flag!=1)
                ProcessEvent(data_prev[data->b]);
            data_prev[data->b]->Clear();
        }
        data->Copy(*data_prev[data->b]);
        //data_prev[data->b] = (NIGIRI*) data->Clone();
    }
}


void decodeV1730dpppha(Packet* p1730dpppha){
    //!**************************************************
    //! v1730DPPPHA packet
    //! *************************************************
    int* tmp;
    int* words;
    words=(int*) p1730dpppha->getIntArray(tmp);
    int totalsize = p1730dpppha->getPadding();
    int pos=0;
    boardaggr_t boardaggr;
    while (pos<totalsize){
        //! board aggr
        //! check if we have valid datum
        if ((words[pos]&0xF0000000)!=0xA0000000) cout<<"Error decoding V1730DPPPHA!"<<endl;
        boardaggr.size = (words[pos]&0xFFFFFFF);
        boardaggr.board_id = (words[pos+1]&0xF8000000)>>27;
        boardaggr.board_fail_flag = (bool)((words[pos+1]&0x4000000)>>26);
        boardaggr.pattern = (words[pos+1]&0x7FFF00)>>8;
        boardaggr.dual_ch_mask = (words[pos+1]&0xFF);
        boardaggr.counter = (words[pos+2]&0x7FFFFF);
        boardaggr.timetag = words[pos+3];
        //boardaggr.print();
        pos+=4;
        for (int i=0;i<V1730_MAX_N_CH/2;i++){
            if (((boardaggr.dual_ch_mask>>i)&0x1)==0) continue;
            //! read channel aggr
            channelaggr_t channelaggr;
            channelaggr.groupid=i;
            channelaggr.formatinfo=(words[pos]&0x80000000)>>31;
            channelaggr.size=words[pos]&0x7FFFFFFF;
            channelaggr.dual_trace_flag=(words[pos+1]&0x80000000)>>31;
            channelaggr.energy_enable_flag=(words[pos+1]&0x40000000)>>30;
            channelaggr.trigger_time_stamp_enable_flag=(words[pos+1]&0x20000000)>>29;
            channelaggr.extra2_enable_flag=(words[pos+1]&0x10000000)>>28;
            channelaggr.waveformsample_enable_flag=(words[pos+1]&0x8000000)>>27;
            channelaggr.extra_option_enable_flag=(words[pos+1]&0x7000000)>>24;
            channelaggr.analogprobe1_selection=(words[pos+1]&0xC00000)>>22;
            channelaggr.analogprobe2_selection=(words[pos+1]&0x300000)>>20;
            channelaggr.digitalprobe_selection=(words[pos+1]&0xF0000)>>16;
            int nsampleto8=(words[pos+1]&0xFFFF);
            channelaggr.n_samples=nsampleto8*8;
            //channelaggr.print();
            pos+=2;

            //! read dual channel data
            //! check if this has two channel data
            int nword_samples=channelaggr.n_samples/2+channelaggr.n_samples%2;
            int nevents=(channelaggr.size-2)/(nword_samples+3);
            for (int i=0;i<nevents;i++){
                channel_t channeldata;
                // read header
                int odd_even_flag=(words[pos]&0x80000000)>>31;
                if (odd_even_flag==0) channeldata.ch=channelaggr.groupid*2;//even
                else channeldata.ch=channelaggr.groupid*2+1;
                channeldata.trigger_time_tag=(words[pos]&0x7FFFFFFF);
                channeldata.n_samples=channelaggr.n_samples;
                pos++;
                // read data samples
                for (int n=0;n<channeldata.n_samples/2+channeldata.n_samples%2;n++){
                    //! read channel samples
                    int ap1= (words[pos]&0x3FFF);
                    int ap2= (words[pos]&0x3FFF0000)>>16;
                    if (channelaggr.dual_trace_flag==1){//dual trace
                        channeldata.ap1_sample.push_back(ap1);//even sample of ap1
                        channeldata.ap2_sample.push_back(ap2);//even sample of ap2
                    }else{//single trace
                        channeldata.ap1_sample.push_back(ap1);//even sample of ap1
                        channeldata.ap1_sample.push_back(ap2);//odd sample of ap1
                        //cout<<"single trace"<<endl;
                    }
                    channeldata.dp_sample.push_back((words[pos]&0x4000)>>14);//even sample
                    channeldata.dp_sample.push_back((words[pos]&0x40000000)>>30);//odd sample
                    channeldata.trg_sample.push_back((words[pos]&0x8000)>>15);//even sample
                    channeldata.trg_sample.push_back((words[pos]&0x80000000)>>31);//odd sample
                    pos++;
                }
                // read extras
                channeldata.extras2 = words[pos];
                channeldata.extras = (words[pos+1]&0x1F0000)>>16;
                channeldata.pileup_rollover_flag = (words[pos+1]&0x8000)>>15;
                channeldata.energy = (words[pos+1]&0x7FFF);
                pos+=2;
                //channeldata.print();
                //! process channel data
                //NIGIRI* data=new NIGIRI;
                data->Clear();
                data->b = p1730dpppha->getHitFormat();
                data->evt = boardaggr.counter;
                data->event_size = channelaggr.size;
                UInt_t tslsb = (UInt_t) (channeldata.trigger_time_tag&0x7FFFFFFF);
                UInt_t tsmsb = (UInt_t) ((channeldata.extras2>>16)&0xFFFF);
                Double_t tpz_baseline = ((Double_t)(channeldata.extras2&0xFFFF))/4.;
                data->ts = (((ULong64_t)tsmsb<<31)&0x7FFF80000000)|(ULong64_t)tslsb;
                data->ts = data->ts*V1730_DGTZ_CLK_RES;
                //cout<<channelaggr.extra2_enable_flag<<"\t"<<channelaggr.extra_option_enable_flag<<"\t"<<data->ts<<endl;
                NIGIRIHit* hit = new NIGIRIHit;
                hit->ch = channeldata.ch;
                hit->clong = channeldata.energy;
                hit->baseline = tpz_baseline;
                //! assign analog and digital probe
                hit->pulse_ap1 =channeldata.ap1_sample;
                hit->pulse_ap2 =channeldata.ap2_sample;
                hit->pulse_dp1 =channeldata.dp_sample;;
                hit->pulse_dp2 =channeldata.trg_sample;
                data->AddHit(hit);
                ProcessEvent(data);
            }//loop on channel data
            //pos+=channelaggr.size;
        }//loop through all  dual channels data
    }//end loop on all words
    //std::cout<<"---"<<std::endl;
}


void decodeV1730dpppsd(Packet* p1730dpppsd){
    //!**************************************************
    //! v1730DPPPSD packet
    //! *************************************************
    int* tmp;
    int* words;
    words=(int*) p1730dpppsd->getIntArray(tmp);
    int totalsize = p1730dpppsd->getPadding();
    int pos=0;
    boardaggr_t boardaggr;
    while (pos<totalsize){
        //! board aggr
        //! check if we have valid datum
        if ((words[pos]&0xF0000000)!=0xA0000000) cout<<"Error decoding V1730DPPPSD!"<<endl;
        boardaggr.size = (words[pos]&0xFFFFFFF);
        boardaggr.board_id = (words[pos+1]&0xF8000000)>>27;
        boardaggr.board_fail_flag = (bool)((words[pos+1]&0x4000000)>>26);
        boardaggr.pattern = (words[pos+1]&0x7FFF00)>>8;
        boardaggr.dual_ch_mask = (words[pos+1]&0xFF);
        boardaggr.counter = (words[pos+2]&0x7FFFFF);
        boardaggr.timetag = words[pos+3];
        //boardaggr.print();
        pos+=4;
        for (int i=0;i<V1730_MAX_N_CH/2;i++){
            if (((boardaggr.dual_ch_mask>>i)&0x1)==0) continue;
            //! read channel aggr
            channelaggr_t channelaggr;
            channelaggr.groupid=i;
            channelaggr.formatinfo=(words[pos]&0x80000000)>>31;
            //changed compared with dpppha
            channelaggr.size=words[pos]&0x3FFFFF;
            channelaggr.dual_trace_flag=(words[pos+1]&0x80000000)>>31;
            channelaggr.energy_enable_flag=(words[pos+1]&0x40000000)>>30;
            channelaggr.trigger_time_stamp_enable_flag=(words[pos+1]&0x20000000)>>29;
            channelaggr.extra2_enable_flag=(words[pos+1]&0x10000000)>>28;
            channelaggr.waveformsample_enable_flag=(words[pos+1]&0x8000000)>>27;
            channelaggr.extra_option_enable_flag=(words[pos+1]&0x7000000)>>24;
            channelaggr.analogprobe1_selection=(words[pos+1]&0xC00000)>>22;
            //changed compared with dpppha
            channelaggr.analogprobe2_selection=(words[pos+1]&0x380000)>>19;
            channelaggr.digitalprobe_selection=(words[pos+1]&0x70000)>>16;
            int nsampleto8=(words[pos+1]&0xFFFF);
            channelaggr.n_samples=nsampleto8*8;
            //channelaggr.print();
            pos+=2;

            //! read dual channel data
            //! check if this has two channel data
            int nword_samples=channelaggr.n_samples/2+channelaggr.n_samples%2;
            int nevents=(channelaggr.size-2)/(nword_samples+3);
            for (int j=0;j<nevents;j++){
                channel_t channeldata;
                // read header
                int odd_even_flag=(words[pos]&0x80000000)>>31;
                if (odd_even_flag==0) channeldata.ch=channelaggr.groupid*2;//even
                else channeldata.ch=channelaggr.groupid*2+1;
                channeldata.trigger_time_tag=(words[pos]&0x7FFFFFFF);
                channeldata.n_samples=channelaggr.n_samples;
                pos++;
                // read data samples
                for (int n=0;n<channeldata.n_samples/2+channeldata.n_samples%2;n++){
                    //! read channel samples
                    int ap1= (words[pos]&0x3FFF);
                    int ap2= (words[pos]&0x3FFF0000)>>16;
                    if (channelaggr.dual_trace_flag==1){//dual trace
                        channeldata.ap1_sample.push_back(ap1);//even sample of ap1
                        channeldata.ap2_sample.push_back(ap2);//odd sample of ap2
                    }else{//single trace
                        channeldata.ap1_sample.push_back(ap1);//even sample of ap1
                        channeldata.ap1_sample.push_back(ap2);//odd sample of ap1
                        //cout<<"single trace"<<endl;
                    }
                    channeldata.dp_sample.push_back((words[pos]&0x4000)>>14);//even sample
                    channeldata.dp_sample.push_back((words[pos]&0x40000000)>>30);//odd sample
                    channeldata.trg_sample.push_back((words[pos]&0x8000)>>15);//even sample
                    channeldata.trg_sample.push_back((words[pos]&0x80000000)>>31);//odd sample
                    pos++;
                }
                // read extras
                //changed compared with dpppha
                channeldata.extras2 = words[pos];
                //changed compared with dpppha
                channeldata.extras = (words[pos+1]&0xFFFF0000)>>16;
                channeldata.pileup_rollover_flag = (words[pos+1]&0x8000)>>15;
                //changed compared with dpppha
                channeldata.energy = (words[pos+1]&0x7FFF);
                pos+=2;
                //channeldata.print();

                //! process channel data
                //NIGIRI* data=new NIGIRI;
                data->Clear();
                data->b = p1730dpppsd->getHitFormat();
                data->evt = boardaggr.counter;
                data->event_size = channelaggr.size;
                UInt_t tslsb = (UInt_t) (channeldata.trigger_time_tag&0x7FFFFFFF);
                UInt_t tsmsb = (UInt_t) ((channeldata.extras2>>16)&0xFFFF);
                //Double_t tpz_baseline = ((Double_t)(channeldata.extras2&0xFFFF))/4.;
                Int_t finets_10bits = (channeldata.extras2&0x3FF);
//                double finets = (Double_t)finets_10bits/1024.*V1730_DGTZ_CLK_RES;
                ULong64_t coarse_ts = (((ULong64_t)tsmsb<<31)&0x7FFF80000000)|(ULong64_t)tslsb;
                data->ts = coarse_ts*V1730_DGTZ_CLK_RES;
                //cout<<channelaggr.extra2_enable_flag<<"\t"<<channelaggr.extra_option_enable_flag<<"\t"<<data->ts<<endl;
                NIGIRIHit* hit = new NIGIRIHit;
                hit->ch = channeldata.ch;
                hit->ts = coarse_ts*1024+finets_10bits;
                hit->finets = (Double_t)(coarse_ts*1024+finets_10bits)*V1730_DGTZ_CLK_RES/1024.;
                //cout<<"hit->finets"<<hit->finets<<endl;
                hit->clong = channeldata.extras;
                hit->cshort = channeldata.energy;
                hit->sampling_interval = channeldata.pileup_rollover_flag;
                //! assign analog and digital probe
                hit->pulse_ap1 =channeldata.ap1_sample;
                hit->pulse_ap2 =channeldata.ap2_sample;
                hit->pulse_dp1 =channeldata.dp_sample;;
                hit->pulse_dp2 =channeldata.trg_sample;
                //if (hit->ch==0) cout<<data->ts<<endl;
                data->AddHit(hit);
                //data->Print();
                ProcessEvent(data);
            }//loop on channel data
        }//loop through all  dual channels data
    }//end loop on all words
//    std::cout<<"---"<<std::endl;
}


void decodelupo(Packet* pLUPO){
    int* tmp;
    int* gg;
    gg=(int*) pLUPO->getIntArray(tmp);
    //NIGIRI* data=new NIGIRI;
    data->Clear();
    data->b = pLUPO->getHitFormat();
    UInt_t tslsb = (UInt_t)gg[3];
    UInt_t tsmsb = (UInt_t)gg[2];
    data->ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;//resolution is 10 ns!
    data->ts = data->ts*LUPO_CLK_RES;
    data->pattern= gg[1];//daq counter
    data->evt = gg[0];
    ProcessEvent(data);
}

int process_event (Event * e)
{
    nevts++;
//    time_t tevt =e->getTime();
//    cout<<"***************** Eventpacket number  = "<<e->getEvtSequence()<<" / record date time"<<asctime(gmtime(&tevt))<<"***************\n"<<endl;
    Packet *pmap[N_PACKETMAP];
    for (Int_t i=0;i<N_PACKETMAP;i++){
        pmap[i]=e->getPacket(packetmap[i]);
        if (pmap[i]){
            if (packetdecode[i]==LUPO){
                decodelupo(pmap[i]);
            }else if (packetdecode[i]==V1740ZSP){
                decodeV1740zsp(pmap[i]);
            }else if (packetdecode[i]==V1740RAW){
                decodeV1740raw(pmap[i]);
            }else if (packetdecode[i]==V1730DPPPHA){
                decodeV1730dpppha(pmap[i]);
            }else if (packetdecode[i]==V1730DPPPSD){
                decodeV1730dpppsd(pmap[i]);
            }else{
                //cout<<"out of definition!"<<endl;
            }
            delete pmap[i];
#ifdef SLOW_ONLINE
    usleep(SLOW_ONLINE);
#endif
        }
    }
    return 0;
}

int pclose(){
    cout<<"Closing"<<endl;
    CloseMe();
    return 0;
}

int phsave (const char *filename){
//    if (filename==0){
//        ts_begin = 0;
//    }else{
//        OpenFile(filename);
//    }
    OpenFile(filename);
    return 0;
}

int main (int argc, char* argv[])
{
    if (argc!=3&&argc!=4){
        cout<<"Usage. ./offline input output"<<endl;
        cout<<"Usage. ./offline input_list_file output anything"<<endl;
        return 0;
    }
    if (argc==3) pfileopen(argv[1]);
    else plistopen(argv[1]);
    phsave(argv[2]);
    prun();
    return 0;
}
