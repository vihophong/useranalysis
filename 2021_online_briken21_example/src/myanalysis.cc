
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

#define V1740_HDR 6
#define V1740_N_CH 64
#define V1740_N_MAX_CH 64
#define NSBL 8
#define N_MAX_WF_LENGTH 120

#define MAX_N_BOARD 16


TH2F *h2;
TH1F *h3wf[V1740_N_MAX_CH];
TH2F *h4wf[V1740_N_MAX_CH];

void Init(){
    h2 = new TH2F("e2d","Energy Spectra 2D",V1740_N_CH*3,0,V1740_N_CH*3,500,0,5000);
//    for (int i=0;i<V1740_N_MAX_CH;i++){
//        h3wf[i] = new TH1F (Form("wf740_1d_%d",i),Form("Waveform 1d v1740 %d",i), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH );
//        h4wf[i] = new TH2F (Form("wf740_2d_%d",i),Form("Waveform 2d v1740 %d",i), N_MAX_WF_LENGTH, 0,N_MAX_WF_LENGTH, 2500, 0,5000 );
//    }
}

void ProcessEvent(NIGIRI* data){
    //data->Print();
    if (data->b==4||data->b==5||data->b==6){
        for (Int_t i=0;i<data->GetMult();i++){
            NIGIRIHit* datahit =data->GetHit(i);
            int ch  =  datahit->ch+(data->b-4)*V1740_N_CH;
            if (datahit->clong>0) {h2->Fill(ch,datahit->clong);}
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
int trigger_pos = 120*0.4-10;// trigger position minus 10 sample

dpp* dppp;


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
  dppp = new dpp;
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
        data->Clear();
        data->DecodeHeaderZsp(gg,p1740zsp->getHitFormat());
        int k=V1740_HDR+V1740_N_MAX_CH;

        //! get number of channels from channel mask

        for (int i=0;i<V1740_N_CH;i++){
          int chgrp = i/8;
          if (((data->channel_mask>>chgrp)&0x1)==0) continue;
          //! header
          NIGIRIHit* chdata=new NIGIRIHit;
          chdata->ch = i;//for sorter
          int nsample = gg[i+V1740_HDR];
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

          if (nsample>NSBL){
              UShort_t adcpos,adc;
              chdata->baseline = dppp->fast_baseline(WaveLine,NSBL);
              dppp->fast_maxADC(adcpos,adc,WaveLine,nsample,trigger_pos);
              chdata->clong = adc-chdata->baseline;
              chdata->finets = adcpos;
          }else{
              chdata->clong = 0;
              chdata->finets = -1;
              chdata->baseline = dppp->fast_baseline(WaveLine,NSBL);
          }
          data->AddHit(chdata);
        }
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
                    data->b = 100;
                    data->evt = boardaggr.counter;
                    data->event_size = channelaggr.size;
                    UInt_t tslsb = (UInt_t) channeldata.trigger_time_tag;
                    UInt_t tsmsb = (UInt_t) channeldata.extras2;
                    data->ts = (((ULong64_t)tsmsb<<32)&0xFFFF00000000)|(ULong64_t)tslsb;
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

