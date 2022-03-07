
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "myanalysis.h"

#include <TH1.h>
#include <TH2.h>

int init_done = 0;

using namespace std;

TH1F *h1;
TH2F *h2;


int pinit()
{

  if (init_done) return 1;
  init_done = 1;

  h1 = new TH1F ( "h1","test histogram", 200, 0, 2000);
  h2 = new TH2F ( "h2","test histogram 2D", 200, 0, 2000, 200, 0, 2000);

  return 0;

}

int process_event (Event * e)
{
  Packet *p = e->getPacket(1001);
  if (p)
    {
      //p->dump(std::cout);
      int* tmp;
      int* word=p->getIntArray(tmp);
      h1->Fill(word[0]);
      h2->Fill(word[0],word[1]);
      delete p;
    }
  return 0;
}

