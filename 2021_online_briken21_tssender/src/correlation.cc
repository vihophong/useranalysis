#include <TF1.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>

#include "correlation.h"
using namespace std;

correlation::correlation(int b1,int b2,Long64_t tw_low, Long64_t tw_high):is_single_corr(true),
    fmax_map_length(20000),fbmap1(b1),fbmap2(b2),ftw_low(tw_low),ftw_high(tw_high),fncorr(0)
{
    map1.clear();
    map2.clear();
    is_self_corr = false;
    map2_mincorr_distance_prev = 0;
}
correlation::correlation(Long64_t tw_low, Long64_t tw_high):is_single_corr(true),
    fmax_map_length(20000),ftw_low(tw_low),ftw_high(tw_high),fncorr(0)
{
    map1.clear();
    map2.clear();
    is_self_corr = true;
    map2_mincorr_distance_prev = 0;
}
correlation::~correlation()
{

}

void correlation::fillMap(bool flagend, NIGIRI* data)
{
    if (map1.size()>fmax_map_length||flagend){
        //cout<<"map"<<endl;
        for(map1_it=map1.begin();map1_it!=map1.end();map1_it++){
            Long64_t ts=(Long64_t)map1_it->first;
            NIGIRI* hit1=(NIGIRI*)map1_it->second;
            Long64_t ts1 = ts - ftw_low;
            Long64_t ts2 = ts + ftw_high;
            Long64_t corrts = 0;
            Int_t ncorr = 0;
            map2_it = map2.lower_bound(ts1);

            int map2_mincorr_distance = 0;
            while(map2_it!=map2.end()&&map2_it->first<ts2){
                corrts = (Long64_t) map2_it->first;
                NIGIRI* hit2=(NIGIRI*)map2_it->second;
                if (is_self_corr&&ts == corrts&&hit1->b==hit2->b&&hit1->b==hit2->b&&hit1->GetHit(0)->ch==hit2->GetHit(0)->ch){//not correlate itself
                    map2_it++;
                    continue;
                }
                //! FILL data for map2 here
                fhcorr1d[0]->Fill(corrts - ts);
                if (hit2->b==hit1->b&&hit2->evt==hit1->evt) {
                    fhcorr1d[1]->Fill(corrts-ts);
//                    if (hit2->ts == hit1->ts){
//                        cout<<hit2->GetHit(0)->finets<<"\t"<<hit1->GetHit(0)->finets<<endl;
//                    }
                }
                if (hit2->b==hit1->b&&hit2->evt!=hit1->evt) {
                    //if (corrts - ts <-576||corrts - ts >576)
                       fhcorr1d[2]->Fill(corrts-ts);
                }

                //! stuff for erasing data after used
                ncorr++;
                if (is_single_corr){//delete all data
                    //! delete already correlated data
                    if (!flagend) {
                        std::multimap<Long64_t,NIGIRI*>::iterator map2_itp;
                        map2_itp=map2_it;
                        map2_itp++;
                        for (std::multimap<Long64_t,NIGIRI*>::iterator map2_ittmp=map2.begin();map2_ittmp!=map2_itp;map2_ittmp++){
                            NIGIRI* hittmp=map2_ittmp->second;
                            hittmp->Clear();
                            delete hittmp;
                        }
                        map2.erase(map2.begin(),map2_itp);
                    }
                    break;//only find first ts match
                }else{
                    if (ncorr == 1){map2_mincorr_distance=std::distance(map2.begin(),map2_it);}

                    map2_it++;
                }//end if single corr
            }//end search on map2
            //! in case of all correlation, only delete from previous datum with min time
//            if (!is_single_corr&&!flagend){
//                if (map2_mincorr_distance!=map2_mincorr_distance_prev&&map2_mincorr_distance>0){
//                    std::multimap<Long64_t,NIGIRI*>::iterator map2_ittmp;
//                    for (map2_ittmp=map2.begin();std::distance(map2.begin(),map2_ittmp)<=map2_mincorr_distance;map2_ittmp++){
//                        NIGIRI* hittmp=map2_ittmp->second;
//                        hittmp->Clear();
//                        delete hittmp;
//                    }
//                    map2.erase(map2.begin(),map2_ittmp);
//                }
//                if (map2_mincorr_distance>0) map2_mincorr_distance_prev = map2_mincorr_distance;
//            }

            //! FILL data for map1 and close event here

            //! progress report
            if (ncorr>0){
                fncorr ++;
                //if (fncorr%10000==0&&fncorr>0) cout<<fncorr<<" correlated events\tMap size = "<<map1.size()<<" - "<<map2.size()<<endl;
            }

            if (!flagend) {//! in all case, delete first entry of map1 after use
                hit1->Clear();
                delete hit1;
                map1.erase(map1.begin(),++map1_it);
                break;//only process earliest datum in the pipe (FIFO)
            }
        }//end for loop all event
    }//end if max map size reached

    //! delete if data size of map2 exceed a given number and issue warning
    if (flagend||map2.size()>fmax_map_length*5){
        //for (map2_it=map2.begin();map2_it!=map2.end();map2_it++){
        for (map2_it=map2.begin();std::distance(map2.begin(),map2_it)<=(long long)(map2.size()-fmax_map_length*5);map2_it++){
            NIGIRI* hittmp=map2_it->second;
            hittmp->Clear();
            delete hittmp;
        }
        //map2.clear();
        map2.erase(map2.begin(),map2_it);
        //if (!flagend) cout<<"Map 2 exceeded size limit, no correlation found!"<<endl;
    }

    //! Continiously fill data for two maps
    if (!flagend){
        if (is_self_corr){
            map1.insert(make_pair(data->ts,data));
            NIGIRI* datac=new NIGIRI;
            data->Copy(*datac);
            map2.insert(make_pair(data->ts,datac));
        }else{
            if (data->b==fbmap1){
                map1.insert(make_pair(data->ts,data));
            }else if(data->b==fbmap2){
                map2.insert(make_pair(data->ts,data));
            }else{
                data->Clear();
                delete data;
            }
        }
    }

    //! if end flag -> clear map
    if (flagend){
        for (map1_it=map1.begin();map1_it!=map1.end();map1_it++){
            NIGIRI* hittmp=map1_it->second;
            hittmp->Clear();
            delete hittmp;
        }
        map1.clear();
    }
}

void correlation::reconstruct()
{

}
