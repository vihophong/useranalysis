
/* ************************ DO NOT REMOVE  ****************************** */
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "myanalysis.h"

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

#define LUPO_PACKET 50
#define V1730_DPPPHA_PACKET 51
#define V1740_PACKET_ZSP 52
#define V1740_PACKET_RAW 53

#define V1730_MAX_N_CH 16
#define V1730DPPPHA_SYNC_CHANNEL 15


#define DGTZ_CLK_RES 8
#define LUPO_CLK_RES 10

//Correlation map
#define MAX_MAP_LENGTH 1000
#define MAX_N_CORR_MAPS 8

std::multimap <ULong64_t,UChar_t> datamap_lupo; //! sort by timestamp
std::multimap <ULong64_t,UChar_t> datamap_dgtz[MAX_N_CORR_MAPS]; //! sort by timestamp
std::multimap<ULong64_t,UChar_t>::iterator it_datamap_lupo;
std::multimap<ULong64_t,UChar_t>::iterator it_datamap_dgtz[MAX_N_CORR_MAPS];

Int_t nlupo;
Int_t ncorr[MAX_N_CORR_MAPS];
Int_t bmap[]= {0,1,2,3,4,5,6,-1};
Long64_t lowerbound[] = {12000,12000,12000,12000,12000,12000,12000,12000};
Long64_t upperbound[] = {12000,12000,12000,12000,12000,12000,12000,12000};


TH1F* hlupodgtz[MAX_N_CORR_MAPS];

MyMainFrame* mf;
TH1F* hrate;
TH1F* hratetmp;
TH1F* hratetmpscale;


void Init(){
    hrate=new TH1F("hrate","hrate",MAX_N_CORR_MAPS,0,MAX_N_CORR_MAPS);
    hratetmp=(TH1F*) hrate->Clone();hratetmp->SetName("hratetmp");
    hratetmpscale=(TH1F*) hrate->Clone();hratetmp->SetName("hratetmpscale");

    nlupo = 0;
    for (Int_t i=0;i<MAX_N_CORR_MAPS;i++){
        ncorr[i] = 0;
        hlupodgtz[i] = new TH1F(Form("hlupodgtz%d",i),Form("hlupodgtz%d",i),2000,-12000,12000);
    }
    mf=new MyMainFrame(gClient->GetRoot(),200,200);
    mf->GetConfig();
    mf->InitHistograms();
    mf->DrawFrameCanvas();
}

void ProcessEvent(NIGIRI* data){
    if (datamap_lupo.size()>MAX_MAP_LENGTH){
        for (it_datamap_lupo=datamap_lupo.begin();it_datamap_lupo!=datamap_lupo.end();it_datamap_lupo++){
            Long64_t ts=(Long64_t)it_datamap_lupo->first;
            Long64_t corrts = 0;

            for (Int_t i=0;i<MAX_N_CORR_MAPS;i++){
                Long64_t ts1 = ts - lowerbound[i];
                ULong64_t ts2 = ts + upperbound[i];
                it_datamap_dgtz[i] = datamap_dgtz[i].lower_bound(ts1);
                while(it_datamap_dgtz[i]!=datamap_dgtz[i].end()&&it_datamap_dgtz[i]->first<ts2){
                    corrts = (Long64_t) it_datamap_dgtz[i]->first;
                    hlupodgtz[i]->Fill(corrts-ts);
                    ncorr[i]++;
                    break;
                }
            }
        }
        datamap_lupo.clear();
        for (Int_t i=0;i<MAX_N_CORR_MAPS;i++){
            datamap_dgtz[i].clear();
        }
    }

    if (data->b<0){
        datamap_lupo.insert(make_pair(data->ts*LUPO_CLK_RES,data->b));
        //cout<<"LUPO\t"<<data->ts*LUPO_CLK_RES<<endl;
    }else{
        if (bmap[data->b]>=0){
            datamap_dgtz[bmap[data->b]].insert(make_pair(data->ts*DGTZ_CLK_RES,data->b));
            //cout<<"DGTZ\t"<<data->ts*DGTZ_CLK_RES<<"\t"<<data->b<<endl;
        }
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


NIGIRI* data;
int init_done = 0;
int n_v1740;
v17xxevent v1740evt;

int pinit()
{
  if (init_done) return 1;
  init_done = 1;
  gROOT->ProcessLine(".L libDataStruct.so");
  data = new NIGIRI;
  Init();
  n_v1740=0;
  return 0;
}

int process_event (Event * e)
{
    //!**************************************************
    //! v1740Raw data packet
    //! *************************************************

    Packet *p1740raw=e->getPacket(V1740_PACKET_RAW);
    if (p1740raw)
    {
#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
        n_v1740++;
        int* tmp;
        int* words;
        words=(int*) p1740raw->getIntArray(tmp);
        int nevt=p1740raw->getPadding();
//        cout<<"\nProcessing events block"<<n_v1740<<" Number of Events="<<nevt<<endl;
        int ipos=0;
        for (int evt=0;evt<nevt;evt++){
            data->DecodeHeaderRaw(words,ipos,p1740raw->getHitFormat());
            ipos+=data->event_size;
            ProcessEvent(data);
        }//end of event loop
        delete p1740raw;
    }//end of packet loop

    //!**************************************************
    //! v1740 with zerosuppresion data packet
    //! *************************************************

    Packet *p1740zsp=e->getPacket(V1740_PACKET_ZSP);
    if (p1740zsp)
    {
#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
        n_v1740++;
        int* temp;
        int* gg;
        gg=(int*) p1740zsp->getIntArray(temp);
        //! header
        data->DecodeHeaderZsp(gg,p1740zsp->getHitFormat());
        ProcessEvent(data);
        delete p1740zsp;
    }


    //!**************************************************
    //! v1730DPPPHA packet
    //! *************************************************
    Packet *p1730dpppha=e->getPacket(V1730_DPPPHA_PACKET);
    if (p1730dpppha)
    {
#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
        int* tmp;
        int* words;
        words=(int*) p1730dpppha->getIntArray(tmp);
        int totalsize = p1730dpppha->getPadding();

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
                    data->b = p1730dpppha->getHitFormat();
                    data->evt = boardaggr.counter;
                    data->event_size = channelaggr.size;
                    UInt_t tslsb = (UInt_t) channeldata.trigger_time_tag;
                    UInt_t tsmsb = (UInt_t) channeldata.extras2;
                    data->ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;
                    if (channeldata.ch == V1730DPPPHA_SYNC_CHANNEL)
                        ProcessEvent(data);
                }//loop on channel data
                //pos+=channelaggr.size;
            }//loop through all  dual channels data

        }//end loop on all words
        //std::cout<<"---"<<std::endl;
        delete p1730dpppha;
    }//end of packet loop

    //!**************************************************
    //! LUPO data packet
    //! *************************************************

    Packet *pLUPO=e->getPacket(LUPO_PACKET);
    if(pLUPO){
#ifdef SLOW_ONLINE
        usleep(SLOW_ONLINE);
#endif
        int* temp;
        int* gg;
        gg=(int*) pLUPO->getIntArray(temp);
        data->Clear();
        data->b = pLUPO->getHitFormat();
        UInt_t tslsb = (UInt_t)gg[3];
        UInt_t tsmsb = (UInt_t)gg[2];
        data->ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;//resolution is 10 ns!
        data->pattern= gg[1];//daq counter
        data->evt = gg[0];
        ProcessEvent(data);
        delete pLUPO;
    }

    return 0;
}

int pclose(){
    CloseMe();
    cout<<"Finish!"<<endl;
    return 0;
}

