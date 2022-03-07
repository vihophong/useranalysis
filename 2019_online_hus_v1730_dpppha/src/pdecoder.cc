
/* ************************ DO NOT REMOVE  ****************************** */
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "pdecoder.h"

#include <dpp.h>
#include <libDataStruct.h>
#include <bitset>
#include <stdint.h>
#include <map>

#include <TSystem.h>
#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TCanvas.h>

#include <fstream>

#include <MyMainFrame.h>
//#include <DataStruct_dict.C>

//#define SLOW_ONLINE 50000000

#define V1730_TRIGGER_CLOCK_RESO 8
#define V1730_TIME_RESO 2
#define V1730_N_CH 16
#define V1730_MAX_N_CH 16
#define V1730_DPPPHA_PACKET 100

#define DIGITAL_PROBE_OFFSET 4000
#define DIGITAL_PROBE_GAIN 1000

#define TRG_PROBE_OFFSET 3000
#define TRG_PROBE_GAIN 1000

#define N_14BITS 16384
using namespace std;

MyMainFrame* mf;

//! histograms and trees
//!
TH1F *hrate;
TH1F *hratetmp;
TH1F *hratetmpscale;

TH1F* hdummy;


TH2F *hap1trace2d[V1730_MAX_N_CH];
TH2F *hap2trace2d[V1730_MAX_N_CH];

TH1F *henergy[V1730_MAX_N_CH];
TH1F *hap1trace1d[V1730_MAX_N_CH];
TH1F *hap2trace1d[V1730_MAX_N_CH];
TH1F *hdptrace1d[V1730_MAX_N_CH];
TH1F *htrgtrace1d[V1730_MAX_N_CH];

typedef struct{
    int size;
    int board_id;
    bool board_fail_flag;
    int pattern;
    int dual_ch_mask;
    int counter;
    int timetag;
    int print(){
        cout<<"board aggregate:"<<endl;
        cout<<"boardaggr_t->size = "<<size<<endl;
        cout<<"boardaggr_t->board_id = "<<board_id<<endl;
        cout<<"boardaggr_t->board_fail_flag = "<<board_fail_flag<<endl;
        cout<<"boardaggr_t->pattern = "<<pattern<<endl;
        cout<<"boardaggr_t->dual_ch_mask = "<<dual_ch_mask<<endl;
        cout<<"boardaggr_t->counter = "<<counter<<endl;
        cout<<"boardaggr_t->timetag = "<<timetag<<endl;
        cout<<"--------------"<<endl;
        return 0;
    }
}boardaggr_t;

typedef struct{
    int groupid;
    bool formatinfo;
    int size;
    bool dual_trace_flag;
    bool energy_enable_flag;
    bool trigger_time_stamp_enable_flag;
    bool extra2_enable_flag;
    bool waveformsample_enable_flag;
    short extra_option_enable_flag;
    short analogprobe1_selection;
    short analogprobe2_selection;
    short digitalprobe_selection;
    int n_samples;
    int print(){
        cout<<"channel aggregate:"<<endl;
        cout<<"channelaggr_t->id = "<<groupid<<endl;
        cout<<"channelaggr_t->formatinfo = "<<formatinfo<<endl;
        cout<<"channelaggr_t->size = "<<size<<endl;
        cout<<"channelaggr_t->dual_trace_flag = "<<dual_trace_flag<<endl;
        cout<<"channelaggr_t->energy_enable_flag = "<<energy_enable_flag<<endl;
        cout<<"channelaggr_t->trigger_time_stamp_enable_flag = "<<trigger_time_stamp_enable_flag<<endl;
        cout<<"channelaggr_t->extra2_enable_flag = "<<extra2_enable_flag<<endl;
        cout<<"channelaggr_t->waveformsample_enable_flag = "<<waveformsample_enable_flag<<endl;
        cout<<"channelaggr_t->extra_option_enable_flag = "<<extra_option_enable_flag<<endl;
        cout<<"channelaggr_t->analogprobe1_selection = "<<analogprobe1_selection<<endl;
        cout<<"channelaggr_t->analogprobe2_selection = "<<analogprobe2_selection<<endl;
        cout<<"channelaggr_t->digitalprobe_selection = "<<digitalprobe_selection<<endl;
        cout<<"channelaggr_t->n_samples = "<<n_samples<<endl;
        cout<<"--------------"<<endl;
        return 0;
    }
}channelaggr_t;
typedef struct{
    int ch;
    int trigger_time_tag;
    int n_samples;
    int extras2;
    int extras;
    bool pileup_rollover_flag;
    int energy;
    vector <int> ap1_sample;
    vector <int> ap2_sample;
    vector <int> dp_sample;
    vector <int> trg_sample;

    int print(){
        cout<<"channel data:"<<endl;
        cout<<"channel_t->ch = "<<ch<<endl;
        cout<<"channel_t->trigger_time_tag = "<<trigger_time_tag<<endl;
        cout<<"channel_t->n_samples = "<<n_samples<<endl;
        cout<<"channel_t->extras2 = "<<extras2<<endl;
        cout<<"channel_t->extras = "<<extras<<endl;
        cout<<"channel_t->pileup_rollover_flag = "<<pileup_rollover_flag<<endl;
        cout<<"channel_t->energy = "<<energy<<endl;
        cout<<"channel_t->ap1_sample_size = "<<ap1_sample.size()<<endl;
        cout<<"channel_t->ap2_sample_size = "<<ap2_sample.size()<<endl;
        cout<<"channel_t->dp_sample_size = "<<dp_sample.size()<<endl;
        cout<<"channel_t->trg_sample_size = "<<trg_sample.size()<<endl;
        cout<<"--------------"<<endl;
        return 0;
    }
}channel_t;

void Init(){
    std::ifstream inpf("channel_calib.txt");
    if (inpf.fail()){
        cout<<"No Configuration table is given"<<endl;
        return;
    }

    hdummy=new TH1F("hdummy","hdummy",10,0,10);
    for (Int_t ch=0;ch<V1730_MAX_N_CH;ch++){
        henergy[ch]=new TH1F(Form("henergy%d",ch),Form("henergy%d",ch),2000,200,40000);
        htrgtrace1d[ch]=new TH1F(Form("htrgtrace1d%d",ch),Form("htrgtrace1d%d",ch),10000,0,20000);
        hdptrace1d[ch]=new TH1F(Form("hdptrace1d%d",ch),Form("hdptrace1d%d",ch),10000,0,20000);
        hap1trace1d[ch]=new TH1F(Form("hap1trace1d%d",ch),Form("hap1trace1d%d",ch),10000,0,20000);
        hap2trace1d[ch]=new TH1F(Form("hap2trace1d%d",ch),Form("hap2trace1d%d",ch),10000,0,20000);
        hap1trace2d[ch]=new TH2F(Form("hap1trace2d%d",ch),Form("hap1trace2d%d",ch),10000,0,20000,1000,-pow(2,13),pow(2,13));
        hap2trace2d[ch]=new TH2F(Form("hap2trace2d%d",ch),Form("hap2trace2d%d",ch),10000,0,20000,1000,-pow(2,13),pow(2,13));
    }

    hrate=new TH1F("hrate","hrate",V1730_N_CH,0,V1730_N_CH);
    hratetmp=new TH1F("hratetmp","hratetmp",V1730_N_CH,0,V1730_N_CH);
    hratetmpscale=new TH1F("hratetmpscale","hratetmpscale",V1730_N_CH,0,V1730_N_CH);

    mf=new MyMainFrame(gClient->GetRoot(),200,200);
    mf->GetConfig();
    mf->InitHistograms();
    mf->DrawFrameCanvas();
}

void ProcessSingleEvent(channelaggr_t* channelaggrdata, channel_t* channeldata){
    if (channelaggrdata->dual_trace_flag){
        hap1trace1d[channeldata->ch]->Reset();
        hap2trace1d[channeldata->ch]->Reset();
        hdptrace1d[channeldata->ch]->Reset();
        htrgtrace1d[channeldata->ch]->Reset();
        for (unsigned long i=0;i<channeldata->ap1_sample.size();i++){
            hap1trace1d[channeldata->ch]->SetBinContent(i*2,channeldata->ap1_sample[i]);
            hap1trace1d[channeldata->ch]->SetBinContent(i*2+1,channeldata->ap1_sample[i]);
            hap1trace2d[channeldata->ch]->Fill(i*2*V1730_TIME_RESO,channeldata->ap1_sample[i]);
            hap1trace2d[channeldata->ch]->Fill((i*2+1)*V1730_TIME_RESO,channeldata->ap1_sample[i]);
        }
        for (unsigned long i=0;i<channeldata->ap2_sample.size();i++){
            if (channeldata->ap2_sample[i]>=N_14BITS/2) channeldata->ap2_sample[i] = channeldata->ap2_sample[i]-N_14BITS;
            hap2trace1d[channeldata->ch]->SetBinContent(i*2,channeldata->ap2_sample[i]);
            hap2trace1d[channeldata->ch]->SetBinContent(i*2+1,channeldata->ap2_sample[i]);
            hap2trace2d[channeldata->ch]->Fill(i*2*V1730_TIME_RESO,channeldata->ap2_sample[i]);
            hap2trace2d[channeldata->ch]->Fill((i*2+1)*V1730_TIME_RESO,channeldata->ap2_sample[i]);
        }
        for (unsigned long i=0;i<channeldata->dp_sample.size();i++){
            hdptrace1d[channeldata->ch]->SetBinContent(i,channeldata->dp_sample[i]*DIGITAL_PROBE_GAIN+DIGITAL_PROBE_OFFSET);
        }
        for (unsigned long i=0;i<channeldata->trg_sample.size();i++){
            htrgtrace1d[channeldata->ch]->SetBinContent(i,channeldata->trg_sample[i]*TRG_PROBE_GAIN+TRG_PROBE_OFFSET);
        }
        henergy[channeldata->ch]->Fill(channeldata->energy);
    }

#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
}

void CloseMe(){

}

//!------------**********************
//! //!------------**********************

//! dont care about the following code
uint16_t seperateint(int inint,bool islsb){
  if (islsb) return (uint16_t) (inint&0xFFFF);
  else return (uint16_t) (inint>>16);
}

int init_done = 0;

int pinit()
{
  if (init_done) return 1;
  init_done = 1;
  gROOT->ProcessLine(".L libDataStruct.so");
  Init();

  return 0;
}

int process_event (Event * e)
{
    //! v1730 packet
    Packet *p1730=e->getPacket(V1730_DPPPHA_PACKET);
    if (p1730)
    {
        int* tmp;
        int* words;
        words=(int*) p1730->getIntArray(tmp);
        int totalsize = p1730->getPadding();

        int pos=0;
        boardaggr_t boardaggr;
        while (pos<totalsize){
            //! board aggr
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
                            cout<<"single trace"<<endl;
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
                    ProcessSingleEvent(&channelaggr,&channeldata);
                }//loop on channel data
                //pos+=channelaggr.size;
            }//loop through all  dual channels data

        }//end loop on all words

        //std::cout<<"---"<<std::endl;
        delete p1730;
    }//end of packet loop
    return 0;
}

int pclose(){
    CloseMe();
    cout<<"Finish!"<<endl;
    return 0;
}

