/**
 * @file thpdf.cxx
 */
  
/* Copyright (C) 2003 Martin Budaj
 * 
 * $Date: $
 * $RCSfile: $
 * $Revision: $
 *
 * -------------------------------------------------------------------- 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * --------------------------------------------------------------------
 */
 
// #include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <string>

// #include <cstring>
#include <cstdio>
#include <cfloat>
#include <cmath>

// #ifdef NOTHERION  
// #include <unistd.h>  // I don't know why
// #endif

#include "thpdfdbg.h"
#include "thpdfdata.h"
#include "thtexfonts.h"

#ifndef NOTHERION
#include "thchenc.h"
#include "thbuffer.h"
#endif

using namespace std;



///////////

// extern list<scraprecord> SCRAPLIST;
// extern map<int,layerrecord> LAYERHASH;
// extern set<int> MAP_PREVIEW_UP, MAP_PREVIEW_DOWN;


///////////

typedef struct {
  int id;
  int namex,namey,layer;
  bool dest,bookmark,title_before;
  map< int,set<string> > scraps;
  int jumpE,jumpW,jumpN,jumpS;
  set<int> jumpU,jumpD;
//  map< int,set<string> > preview;
//  set<string> preview;
} sheetrecord;

map<string,set<string> > preview;
map<string,list<sheetrecord>::iterator> SHEET_JMP;


bool operator < (sheetrecord a, sheetrecord b) {
  return a.id < b.id;
}

bool operator == (sheetrecord a, sheetrecord b) {
  return a.id == b.id;
}

list<sheetrecord> SHEET;


//////////

int mode;
const int ATLAS = 0, MAP = 1;
double MINX=DBL_MAX, MINY=DBL_MAX, MAXX=-DBL_MAX, MAXY=-DBL_MAX;
double HS,VS;
//////////

string tex_Sname(string s) {return("THS"+s);}
string tex_Nname(string s) {return("THN"+s);}

void read_settings() {
  ifstream F("scraps.dat");
  if(!F) therror(("Can't open file `scraps.dat'!"));
  char buf[101],ctok[100];
  string tok;
  int context = 0;   // 0=scrap, 1=layer, 2=map preview, 3=legend
  int i;
  double llx,lly,urx,ury;
  scraprecord SCR;
  layerrecord LH;
  list<scraprecord>::iterator I;
  map<int,layerrecord>::iterator J;

  while(F.getline(buf,100,'\n')) {
//    istrstream S(buf);
    istringstream S(buf);
    S >> ctok;
    tok = ctok;
    if      (tok == "[SCRAP]")  context = 0;
    else if (tok == "[LAYER]")  context = 1;
    else if (tok == "[MAP]")    context = 2;
    else if (tok == "[LEGEND]") context = 3;
    else if (tok == "N") {
      switch (context) {
        case 0:
          S >> ctok;
          SCRAPLIST.push_front(SCR);
          I = SCRAPLIST.begin();
          I->name = ctok;
          break;
        case 1:
          S >> ctok; ((*J).second).N = ctok;
          while(S >> ctok) {
            ((*J).second).N = ((*J).second).N + " " + string(ctok);
          }
          break;
      }
    }
    else if (tok == "F") {
      S >> llx >> lly >> urx >> ury;
      I->F = " "; I->F1 = llx; I->F2 = lly; I->F3 = urx; I->F4 = ury;
    }
    else if (tok == "G") {
      S >> llx >> lly >> urx >> ury;
      I->G = " "; I->G1 = llx; I->G2 = lly; I->G3 = urx; I->G4 = ury;
    }
    else if (tok == "B") {
      S >> llx >> lly >> urx >> ury;
      I->B = " "; I->B1 = llx; I->B2 = lly; I->B3 = urx; I->B4 = ury;
    }
    else if (tok == "I") {
      S >> llx >> lly >> urx >> ury;
      I->I = " "; I->I1 = llx; I->I2 = lly; I->I3 = urx; I->I4 = ury;
    }
    else if (tok == "E") {
      S >> llx >> lly >> urx >> ury;
      I->E = " "; I->E1 = llx; I->E2 = lly; I->E3 = urx; I->E4 = ury;
    }
    else if (tok == "X") {
      S >> llx >> lly >> urx >> ury;
      I->X = " "; I->X1 = llx; I->X2 = lly; I->X3 = urx; I->X4 = ury;
    }
    else if (tok == "P") {
      S >> ctok;  I->P = ctok;  
      S >> llx >> lly;  I->S1 = llx; I->S2 = lly;
    }
    else if (tok == "Y") {
      S >> i;  I->layer = i;
    }
    else if (tok == "V") {
      S >> i;  I->level = i;
    }
    else if (tok == "R") {
      S >> i;
      LAYERHASH.insert(make_pair(i,LH));
      J = LAYERHASH.find(i);
      ((*J).second).Z = 0;
    }
    else if (tok == "U") {
      switch (context) {
        case 1:
          while(S >> i) {
            (((*J).second).U).insert(i);
          }
          break;
        case 2:
          while(S >> i) {
            MAP_PREVIEW_UP.insert(i);
          }
          break;
      }
    }
    else if (tok == "D") {
      switch (context) {
        case 1:
          while(S >> i) {
            (((*J).second).D).insert(i);
          }
          break;
        case 2:
          while(S >> i) {
            MAP_PREVIEW_DOWN.insert(i);
          }
          break;
      }
    }
    else if (tok == "T") {
      S >> ctok; ((*J).second).T = ctok;
      while(S >> ctok) {
        ((*J).second).T = ((*J).second).T + " " + string(ctok);
      }
    }
    else if (tok == "Z") {
      switch (context) {
        case 0:
          S >> i;  I->sect = i;
          break;
        case 1:
          ((*J).second).Z = 1;
          break;
      }
    }
  }
  F.close();
  SCRAPLIST.reverse();
}

string xyz2str(int x, int y, int z) {
  char buf[50];
  sprintf(buf,"%d.%d.%d",x,y,z);
  return (string) buf;
}

list<sheetrecord>::iterator find_sheet(int x, int y, int z) {
//  list<sheetrecord>::iterator I;
//  for (I = SHEET.begin(); I != SHEET.end(); I++) {
//      if (I->layer == x && I->namex == y && I->namey == z) break;
//    }
//    return (I);
  if (SHEET_JMP.count(xyz2str(x,y,z)) > 0) 
    return SHEET_JMP.find(xyz2str(x,y,z))->second;
  else return SHEET.end();
}

void make_sheets() {
  double llx = 0, lly = 0, urx = 0, ury = 0;
  sheetrecord SHREC;

  if (mode == ATLAS) {
    for (map<int,layerrecord>::iterator I = LAYERHASH.begin(); 
                                        I != LAYERHASH.end(); I++) {
      I->second.minx = INT_MAX;
      I->second.miny = INT_MAX;
      I->second.maxx = INT_MIN;
      I->second.maxy = INT_MIN;
    }
  }
  for (list<scraprecord>::iterator I = SCRAPLIST.begin(); 
                                  I != SCRAPLIST.end(); I++) {
    llx = DBL_MAX; lly = DBL_MAX; urx = -DBL_MAX; ury = -DBL_MAX;
    if (I->F != "") {
      if (I->F1 < llx) llx = I->F1;
      if (I->F2 < lly) lly = I->F2;
      if (I->F3 > urx) urx = I->F3;
      if (I->F4 > ury) ury = I->F4;
    }
    if (I->E != "") {
      if (I->E1 < llx) llx = I->E1;
      if (I->E2 < lly) lly = I->E2;
      if (I->E3 > urx) urx = I->E3;
      if (I->E4 > ury) ury = I->E4;
    }
    if (I->G != "") {
      if (I->G1 < llx) llx = I->G1;
      if (I->G2 < lly) lly = I->G2;
      if (I->G3 > urx) urx = I->G3;
      if (I->G4 > ury) ury = I->G4;
    }
    if (I->B != "") {
      if (I->B1 < llx) llx = I->B1;
      if (I->B2 < lly) lly = I->B2;
      if (I->B3 > urx) urx = I->B3;
      if (I->B4 > ury) ury = I->B4;
    }
    if (I->I != "") {
      if (I->I1 < llx) llx = I->I1;
      if (I->I2 < lly) lly = I->I2;
      if (I->I3 > urx) urx = I->I3;
      if (I->I4 > ury) ury = I->I4;
    }
    if (I->X != "") {
      if (I->X1 < llx) llx = I->X1;
      if (I->X2 < lly) lly = I->X2;
      if (I->X3 > urx) urx = I->X3;
      if (I->X4 > ury) ury = I->X4;
    }
    
    if (llx == DBL_MAX || lly == DBL_MAX || urx == -DBL_MAX || ury == -DBL_MAX) 
      therror(("This can't happen -- no data for a scrap!"));
    
    map<int,layerrecord>::iterator J = LAYERHASH.find(I->layer);
    if (J == LAYERHASH.end()) therror (("This can't happen!"));

    if (mode == ATLAS) {
      int Llx = (int) floor((llx-LAYOUT.overlap-LAYOUT.hoffset) / LAYOUT.hsize);
      int Lly = (int) floor((lly-LAYOUT.overlap-LAYOUT.voffset) / LAYOUT.vsize);
      int Urx = (int) floor((urx+LAYOUT.overlap-LAYOUT.hoffset) / LAYOUT.hsize);
      int Ury = (int) floor((ury+LAYOUT.overlap-LAYOUT.voffset) / LAYOUT.vsize);

      for (int i = Llx; i <= Urx; i++) {
        for (int j = Lly; j <= Ury; j++) {
          if (J->second.Z == 0) {    // Z layers don't create new sheets
            list<sheetrecord>::iterator sheet_it = find_sheet(I->layer,i,j);
            if (sheet_it == SHEET.end()) {
              sheet_it = SHEET.insert(SHEET.end(),SHREC);
              SHEET_JMP.insert(make_pair(xyz2str(I->layer,i,j),sheet_it));
            }
            sheet_it->layer = I->layer;
            sheet_it->namex = i;
            sheet_it->namey = j;

            map<int,set<string> >::iterator K = sheet_it->scraps.find(I->level);
            if (K == sheet_it->scraps.end()) {
              set<string> SCRP;
              sheet_it->scraps.insert(make_pair(I->level,SCRP));
              K = sheet_it->scraps.find(I->level);
            }
            ((*K).second).insert(I->name);
        
            if (J->second.minx > Llx) J->second.minx = Llx;
            if (J->second.miny > Lly) J->second.miny = Lly;
            if (J->second.maxx < Urx) J->second.maxx = Urx;
            if (J->second.maxy < Ury) J->second.maxy = Ury;
          } 
          // we add scrap to preview
          set<string> tmpset;
          string tmpstr;
          tmpstr = xyz2str(I->layer,i,j);
          map<string,set<string> >::iterator pr_it = preview.find(tmpstr);
          if (pr_it == preview.end()) {
            preview.insert(make_pair(tmpstr,tmpset));
            pr_it = preview.find(tmpstr);
          }
          pr_it->second.insert(I->name);
        }
      }
    }
    else {
      map<int,set<string> >::iterator K = (((*J).second).scraps).find(I->level);
      if (K == (((*J).second).scraps).end()) {
        set<string> SCRP;
        (((*J).second).scraps).insert(make_pair(I->level,SCRP));
        K = (((*J).second).scraps).find(I->level);
      }
      ((*K).second).insert(I->name);
      
      (((*J).second).allscraps).insert(I->name);
      
      if (((*J).second).Z == 0) {
        if (MINX > llx) MINX = llx;
        if (MINY > lly) MINY = lly;
        if (MAXX < urx) MAXX = urx;
        if (MAXY < ury) MAXY = ury;
      }
    }

  }
//  cout << "MINMAX " << MINX << " " << MINY << " " << MAXX << " " << MAXY << endl;
}

void find_jumps() {
  for (list<sheetrecord>::iterator sheet_it = SHEET.begin(); 
                                   sheet_it != SHEET.end(); sheet_it++) {
    sheet_it->jumpW = 0;
    sheet_it->jumpE = 0;
    sheet_it->jumpN = 0;
    sheet_it->jumpS = 0;

    list<sheetrecord>::iterator I;
    
    int jump;    
                               
    string W = xyz2str(sheet_it->layer,sheet_it->namex-1,sheet_it->namey);
    string E = xyz2str(sheet_it->layer,sheet_it->namex+1,sheet_it->namey);
    string N = xyz2str(sheet_it->layer,sheet_it->namex,sheet_it->namey+1);
    string S = xyz2str(sheet_it->layer,sheet_it->namex,sheet_it->namey-1);
    if (SHEET_JMP.count(W) > 0) {
       I = SHEET_JMP.find(W)->second;
       sheet_it->jumpW = I->id;
       I->dest = true;
    }
    if (SHEET_JMP.count(E) > 0) {
       I = SHEET_JMP.find(E)->second;
       sheet_it->jumpE = I->id;
       I->dest = true;
    }
    if (SHEET_JMP.count(N) > 0) {
       I = SHEET_JMP.find(N)->second;
       sheet_it->jumpN = I->id;
       I->dest = true;
    }
    if (SHEET_JMP.count(S) > 0) {
       I = SHEET_JMP.find(S)->second;
       sheet_it->jumpS = I->id;
       I->dest = true;
    }

    map<int,layerrecord>::iterator lay_it = LAYERHASH.find(sheet_it->layer);
    if (lay_it == LAYERHASH.end()) therror (("This can't happen!"));

    if (!lay_it->second.U.empty()) {
      for (set<int>::iterator l_it = lay_it->second.U.begin(); 
                              l_it != lay_it->second.U.end(); l_it++) {
        map<int,layerrecord>::iterator alt_lay_it = LAYERHASH.find(*l_it);
        if (alt_lay_it == LAYERHASH.end()) therror(("This can't happen!"));
        jump = (alt_lay_it->second.Z == 0) ? *l_it : alt_lay_it->second.AltJump;
        string U = xyz2str(jump,sheet_it->namex,sheet_it->namey);
        if (SHEET_JMP.count(U) > 0) {
          I = SHEET_JMP.find(U)->second;
          sheet_it->jumpU.insert(jump);
          I->dest = true;
        }
      }
    }
    if (!lay_it->second.D.empty()) {
      for (set<int>::iterator l_it = lay_it->second.D.begin(); 
                              l_it != lay_it->second.D.end(); l_it++) {
        map<int,layerrecord>::iterator alt_lay_it = LAYERHASH.find(*l_it);
        if (alt_lay_it == LAYERHASH.end()) therror(("This can't happen!"));
        jump = (alt_lay_it->second.Z == 0) ? *l_it : alt_lay_it->second.AltJump;
        string D = xyz2str(jump,sheet_it->namex,sheet_it->namey);
        if (SHEET_JMP.count(D) > 0) {
          I = SHEET_JMP.find(D)->second;
          sheet_it->jumpD.insert(jump);
          I->dest = true;
        }
      }
    }
  }
}




string grid_name(string s, int offset) {
  unsigned char c;
  bool is_num = true;

  for (unsigned i=0; i<s.size(); i++) {
    c=s[i];
    if (!isdigit(c)) {
      is_num = false;
      break;
    }
  }
  if (is_num) {
    char buf[10];
    sprintf(buf,"%d",atoi(s.c_str())+offset);
    return (string) buf;
  }
  else if (s.size()==1) {
    c=s[0];
    if ((c >= 65 && c <= 90 && (c+offset) >= 65 && (c+offset) <= 90) ||
        (c >= 97 && c <=122 && (c+offset) >= 97 && (c+offset) <=122)) {
      char buf[10];
      sprintf(buf,"%c",c+offset);
      return (string) buf;
    }
    else return "?";
  }
  else return "?";
}

set<int> find_excluded_pages(string s) {
  set<int> excl;
  int i,j;
  char c;
//  istrstream S(s.c_str());
  istringstream S(s);
  
  while (S >> i) {
    S >> c;
    if (c == ',') excl.insert(i);
    else if (c == '-') {
      S >> j;
      for (int k=i; k<=j; k++) excl.insert(k);
      S >> c;   // punctuation character
    }
    else therror(("Invalid character in the exclusion list!"));
  }
//cout << endl;
//cout << "Excl.list: " << s << endl;  
//cout << "Excl. set: ";  
//for (set<int>::iterator I = excl.begin(); I != excl.end(); I++) 
//  cout << *I << " ";
//cout << endl;
  return excl;
}

void sort_sheets() {
  int pageid = 1 + LAYOUT.own_pages, tmppagenum = 1;
  set<int> excluded;
  bool wait_for_title;
  
  if (LAYOUT.excl_pages) excluded = find_excluded_pages(LAYOUT.excl_list);

  for (map<int,layerrecord>::reverse_iterator I = LAYERHASH.rbegin(); 
                                      I != LAYERHASH.rend(); I++) {
//    I->second.minid = pageid;
    I->second.bookmark = false;
    wait_for_title = (I->second.T !="" && LAYOUT.title_pages) ? true : false;

    for (int j = I->second.maxy; j >= I->second.miny; j--) {
      for (int i = I->second.minx; i <= I->second.maxx; i++) {
        list<sheetrecord>::iterator sheet_it = find_sheet(I->first,i,j);
        if (sheet_it != SHEET.end()) {
//          if (wait_for_title && excluded.count(tmppagenum) > 0) {
//            wait_for_title = false;
//          }
          if (excluded.count(tmppagenum) == 0) {
            sheet_it->dest = false;
            sheet_it->title_before = false;
            sheet_it->bookmark = false;
            if (!I->second.bookmark) {
              sheet_it->bookmark = true;
              sheet_it->dest = true;
              I->second.bookmark = true;
            }
            if (wait_for_title) {
              sheet_it->title_before = true;
              pageid++;
              wait_for_title = false;
            }
            sheet_it->id = pageid;
            pageid++;
          }
          else {
            SHEET.erase(sheet_it);
            SHEET_JMP.erase(xyz2str(sheet_it->layer,
                                    sheet_it->namex,sheet_it->namey));
//cout << "Should erase sheet " << tmppagenum << endl;
          }
          tmppagenum++;
        }
      }
    }
//    I->second.maxid = pageid - 1;
  }
  SHEET.sort();
//cout << "sheets: " << SHEET.size() << endl;  
}


void print_preview(int up,ofstream& PAGEDEF,double HSHIFT,double VSHIFT,
                   list<sheetrecord>::iterator sheet_it) {
  set<int> used_layers;
  set<string> used_scraps;
  double xc = 0, yc = 0;
  
  if (LAYOUT.OCG) {
    PAGEDEF << "\\setbox\\xxx=\\hbox to " << HS << "bp{%" << endl;
  }

  PAGEDEF << (up ? "\\PL{q .1 w}%" : "\\PL{q .8 g}%") << endl;
  if (mode == ATLAS) {
    map<int,layerrecord>::iterator lay_it = LAYERHASH.find(sheet_it->layer);
    if (lay_it == LAYERHASH.end()) therror(("This can't happen!"));
    used_layers = (up ? lay_it->second.U : lay_it->second.D);
  }
  else {
    used_layers = (up ? MAP_PREVIEW_UP : MAP_PREVIEW_DOWN);
  }
  for (set<int>::iterator I=used_layers.begin(); I != used_layers.end(); I++) {
    if (mode == ATLAS) {
        map<string,set<string> >::iterator pr_it = 
          preview.find(xyz2str(*I,sheet_it->namex,sheet_it->namey));
        if (pr_it != preview.end()) used_scraps = pr_it->second;
    }
    else {
      map<int,layerrecord>::iterator J = LAYERHASH.find(*I);
      if (J == LAYERHASH.end()) therror(("This can't happen!"));
      used_scraps = J->second.allscraps;
    }
    if (!used_scraps.empty()) {
      for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
        if (used_scraps.count(K->name) > 0) {
          if (up) {
            if (K->B != "" && K->sect == 0) {
              xc = K->B1; yc = K->B2;
              xc -= HSHIFT; yc -= VSHIFT;
              PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                      tex_Xname("B"+(K->name)) << "}%" << endl;
            }
          }
          else {
            if (K->I != "" && K->sect == 0) {
              xc = K->I1; yc = K->I2;
              xc -= HSHIFT; yc -= VSHIFT;
              PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                      tex_Xname("I"+(K->name)) << "}%" << endl;
            }
          }
        }
      }
    }
  }
  PAGEDEF << "\\PL{Q}%" << endl;
  if (LAYOUT.OCG) {
    PAGEDEF << "\\hfill}\\ht\\xxx=" << VS << "bp\\dp\\xxx=0bp" << endl;
    PAGEDEF << "\\immediate\\pdfxform ";
    PAGEDEF << "attr{/OC \\the\\" << (up ? "ocU" : "ocD") << "\\space 0 R} ";
    PAGEDEF << "\\xxx\\PB{0}{0}{\\pdflastxform}%" << endl;
    
  }
}



void compose_page(list<sheetrecord>::iterator sheet_it, ofstream& PAGE) {
  map<int,layerrecord>::iterator lay_it = LAYERHASH.find(sheet_it->layer);
  if (lay_it == LAYERHASH.end()) therror (("This can't happen!"));

  if (sheet_it->title_before) {
    PAGE << "\\TITLE{" << utf2tex(lay_it->second.T) << "}\n";
  }

  PAGE << "%\n% Page: " << u2str(sheet_it->id) << endl << "%\n";
  if (sheet_it->dest) PAGE << "\\pdfdest name {" << u2str(sheet_it->id) << 
                              "} xyz" << endl;

  if (sheet_it->bookmark) {
    PAGE << "\\pdfoutline goto name {" << u2str(sheet_it->id) << 
                  "} count 0 {\\ne\\376\\ne\\377" << 
                  utf2texoctal(lay_it->second.N) << "}%" << endl;
  }
  PAGE << "\\setbox\\mapbox=\\hbox to " << HS << "bp{%" << endl;
  PAGE << "\\rlap{\\pdfrefxform\\" << tex_Sname(u2str(sheet_it->id)) << 
          "}%" << endl;

  // map hyperlinks
  int lw = 25;
  double lhy = VS - 2*lw;
  double lhx = HS - 2*lw;
  if (sheet_it->jumpW) PAGE << "\\flatlink{0}{" << lw << "}{" << lw << 
          "}{" << lhy << "}{" << u2str(sheet_it->jumpW) << "}%\n";
  if (sheet_it->jumpE) PAGE << "\\flatlink{" << HS-lw << "}{" << lw <<
          "}{" << lw << "}{" << lhy << "}{" << u2str(sheet_it->jumpE) << "}%\n";
  if (sheet_it->jumpN) PAGE << "\\flatlink{" << lw << "}{" << VS-lw << 
          "}{" << lhx << "}{" << lw << "}{" << u2str(sheet_it->jumpN)<< "}%\n";
  if (sheet_it->jumpS) PAGE << "\\flatlink{" << lw << "}{0}{" << lhx << 
          "}{" << lw << "}{" << u2str(sheet_it->jumpS) << "}%\n";
  

  PAGE << "\\hfil}\\ht\\mapbox=" << VS << "bp%" << endl;

  PAGE << "\\pagelabel={" << grid_name(LAYOUT.labely,-sheet_it->namey) << 
                        " " << grid_name(LAYOUT.labelx,sheet_it->namex) <<
                        "}%" << endl;
  if (LAYOUT.page_numbering) PAGE << "\\pagenum=" << 
                             sheet_it->id << "%" << endl;
    
//    up and down links

  if (!sheet_it->jumpU.empty()) {
    PAGE << "\\pointerU={%\n";
    for (set<int>::reverse_iterator l_it = sheet_it->jumpU.rbegin();
                                    l_it != sheet_it->jumpU.rend(); l_it++) {
      list<sheetrecord>::iterator s_it = 
        find_sheet(*l_it,sheet_it->namex,sheet_it->namey);
      if (s_it == SHEET.end()) therror (("This can't happen!"));
      PAGE << utf2tex(LAYERHASH.find(*l_it)->second.N) << "|" <<
           s_it->id << "|" << u2str(s_it->id) << "||%" << endl;
    }
    PAGE << "}%\n";
  }
  else PAGE << "\\pointerU={notdef}%" << endl;

  if (!sheet_it->jumpD.empty()) {
    PAGE << "\\pointerD={%\n";
    for (set<int>::reverse_iterator l_it = sheet_it->jumpD.rbegin();
                                    l_it != sheet_it->jumpD.rend(); l_it++) {
      list<sheetrecord>::iterator s_it = 
        find_sheet(*l_it,sheet_it->namex,sheet_it->namey);
      if (s_it == SHEET.end()) therror (("This can't happen!"));
      PAGE << utf2tex(LAYERHASH.find(*l_it)->second.N) << "|" <<
           s_it->id << "|" << u2str(s_it->id) << "||%" << endl;
    }
    PAGE << "}%\n";
  }
  else PAGE << "\\pointerD={notdef}%" << endl;

  PAGE << "\\pagename={" << utf2tex(lay_it->second.N) << "}%" << endl;

  // pointers to neighbouring pages
  if (LAYOUT.page_numbering) {
    if (sheet_it->jumpW) PAGE << "\\pointerW=" << sheet_it->jumpW << "%\n";
    else PAGE << "\\pointerW=0%\n";
    if (sheet_it->jumpE) PAGE << "\\pointerE=" << sheet_it->jumpE << "%\n";
    else PAGE << "\\pointerE=0%\n";
    if (sheet_it->jumpN) PAGE << "\\pointerN=" << sheet_it->jumpN << "%\n";
    else PAGE << "\\pointerN=0%\n";
    if (sheet_it->jumpS) PAGE << "\\pointerS=" << sheet_it->jumpS << "%\n";
    else PAGE << "\\pointerS=0%\n";
  }

  PAGE << "\\setbox\\navbox=\\hbox{%" << endl;
  // navigator hyperlinks
  int nav_x = 2*LAYOUT.nav_right+1;
  int nav_y = 2*LAYOUT.nav_up+1;
  double HSN = LAYOUT.hsize / LAYOUT.nav_factor * nav_x;
  double VSN = LAYOUT.vsize / LAYOUT.nav_factor * nav_y;
  for (int i=-LAYOUT.nav_right; i <= LAYOUT.nav_right; i++) {
    for (int j=-LAYOUT.nav_up; j <= LAYOUT.nav_up; j++) {
      if (i!=0 || j!=0) {
        string tmp = xyz2str(sheet_it->layer,
                             sheet_it->namex+i,sheet_it->namey+j);
        if (SHEET_JMP.count(tmp) > 0) {
          PAGE << "\\flatlink{" << HSN*(i+LAYOUT.nav_right)/nav_x <<
            "}{" << VSN*(j+LAYOUT.nav_up)/nav_y << "}{" <<
            HSN/nav_x << "}{" << VSN/nav_y << "}{" << 
            u2str(SHEET_JMP.find(tmp)->second->id) << "}%\n";
        }
      }
    }
  }
  
  PAGE << "\\pdfrefxform\\" << tex_Nname(u2str(sheet_it->id)) << "}%" << endl;

  PAGE << "\\dopage\\eject" << endl;
}

void print_map(int layer, ofstream& PAGEDEF, 
               list<sheetrecord>::iterator sheet_it){
  double HSHIFT=0, VSHIFT=0, xc = 0, yc = 0;
  map < int,set<string> > LEVEL;
  set <string> page_text_scraps,used_scraps;
  string buffer;
  deque<string> thstack;

  if (mode == ATLAS) {
    HSHIFT = LAYOUT.hsize * sheet_it->namex + LAYOUT.hoffset - LAYOUT.overlap;
    VSHIFT = LAYOUT.vsize * sheet_it->namey + LAYOUT.voffset - LAYOUT.overlap; 
    LEVEL = sheet_it->scraps;
  }
  else {
    HSHIFT = MINX;
    VSHIFT = MINY;
    LEVEL = ((*(LAYERHASH.find(layer))).second).scraps;
  }

  for (map < int,set<string> >::iterator I = LEVEL.begin();
                                         I != LEVEL.end(); I++) {
    used_scraps = (*I).second;
    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
       if (used_scraps.count(K->name) > 0 && K->P != "") 
          page_text_scraps.insert(K->name);
    }
  }
  
  if (mode == ATLAS && !LAYERHASH.find(layer)->second.D.empty()) {
    print_preview(0,PAGEDEF,HSHIFT,VSHIFT,sheet_it);
  }
  
  for (map < int,set<string> >::iterator I = LEVEL.begin();
                                         I != LEVEL.end(); I++) {
    used_scraps = (*I).second;

    if (LAYOUT.transparency) {                 // transparency group beginning
      PAGEDEF << "\\setbox\\xxx=\\hbox to " << HS << "bp{%" << endl;
      PAGEDEF << "\\PL{/GS1 gs}%" << endl;     // beginning of transparency
    }

    PAGEDEF << "\\PL{q 1 g}%" << endl;         // white background of the scrap
    
    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
      if (used_scraps.count(K->name) > 0 && K->I != "") {
        xc = K->I1; yc = K->I2;
        xc -= HSHIFT; yc -= VSHIFT;
        PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                tex_Xname("I"+(K->name)) << "}%" << endl;
      }
    }
    PAGEDEF << "\\PL{Q}%" << endl;            // end of white color for filled bg

    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
      if (used_scraps.count(K->name) > 0 && K->G != "") {
        xc = K->G1; yc = K->G2;
        xc -= HSHIFT; yc -= VSHIFT;
        PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                tex_Xname("G"+(K->name)) << "}%" << endl;
      };
    }

    if (LAYOUT.transparency) {
      PAGEDEF << "\\PL{/GS0 gs}%" << endl;      // end of default transparency
    }

    PAGEDEF << "\\PL{q 0 0 m " << HS << " 0 l " << HS << " " << 
                                  VS << " l 0 " << VS << " l 0 0 l}";
                               // beginning of the text clipping path definition

    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
      if (page_text_scraps.count(K->name) > 0 && 
          K->P != "" && K->level >= (I->first)) {
        xc = HSHIFT - K->S1; yc = VSHIFT - K->S2;
        ifstream G((K->P).c_str());
        if(!G) therror(("Can't open file"));
        while(G >> buffer) {
          if ((buffer == "m") || (buffer == "l") || (buffer == "c")) {
            PAGEDEF << "\\PL{"; 
            for(unsigned i=0; i<thstack.size(); i=i+2) {
              PAGEDEF << atof(thstack[i].c_str())-xc << " " << 
                         atof(thstack[i+1].c_str())-yc << " ";
            }
            PAGEDEF << buffer << "}%" << endl;
            thstack.clear();
          }
          else {
            thstack.push_back(buffer);
          }
        }
        G.close();
        if (!thstack.empty()) therror(("This can't happen -- bad text clipping path!"));
      };
    }
  
    PAGEDEF << "\\PL{h W n}";  // end of text clipping path definition
    
    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
      if (used_scraps.count(K->name) > 0 && K->F != "") {
        xc = K->F1; yc = K->F2;
        xc -= HSHIFT; yc -= VSHIFT;
        PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                tex_Xname(K->name) << "}%" << endl;
      };
    }
   
    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
      if (used_scraps.count(K->name) > 0 && K->E != "") {
        xc = K->E1; yc = K->E2;
        xc -= HSHIFT; yc -= VSHIFT;
        PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                tex_Xname("E"+(K->name)) << "}%" << endl;
      };
    }
   
    PAGEDEF << "\\PL{Q}";   // end of clipping by text

    for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
      if (used_scraps.count(K->name) > 0 && K->X != "") {
        xc = K->X1; yc = K->X2;
        xc -= HSHIFT; yc -= VSHIFT;
        PAGEDEF << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                tex_Xname("X"+(K->name)) << "}%" << endl;
      };
    }
    if (LAYOUT.transparency) {
      PAGEDEF << "\\hfill}\\ht\\xxx=" << VS << "bp\\dp\\xxx=0bp" << endl;
      PAGEDEF << "\\immediate\\pdfxform ";
      PAGEDEF << "attr{/Group \\the\\attrid\\space 0 R} ";
      PAGEDEF << "resources{/ExtGState \\the\\resid\\space 0 R}";
      PAGEDEF << "\\xxx\\PB{0}{0}{\\pdflastxform}%" << endl;
    }
  }
  
  if (mode == ATLAS && !LAYERHASH.find(layer)->second.U.empty()) {
    print_preview(1,PAGEDEF,HSHIFT,VSHIFT,sheet_it);
  }
}

void print_navigator(ofstream& P, list<sheetrecord>::iterator sheet_it) {
  set<string> NAV_SCRAPS;
  set<int> used_layers;
  set<string> used_scraps;
  int nav_x = 2*LAYOUT.nav_right+1;
  int nav_y = 2*LAYOUT.nav_up+1;
  double HSN = LAYOUT.hsize / LAYOUT.nav_factor * nav_x;
  double VSN = LAYOUT.vsize / LAYOUT.nav_factor * nav_y;
  double xc = 0, yc = 0;

  P << "%\n\\setbox\\xxx=\\hbox to " << HSN << "bp{%\n\\PL{q ";
  P.precision(6);
  P << 1/LAYOUT.nav_factor << " 0 0 " << 1/LAYOUT.nav_factor << " 0 0 cm}%\n";
  P.precision(1);

  map<int,layerrecord>::iterator lay_it = LAYERHASH.find(sheet_it->layer);
  if (lay_it == LAYERHASH.end()) therror (("This can't happen!"));

  NAV_SCRAPS.clear();
  if (!lay_it->second.D.empty()) {
    P << "\\PL{.8 g}%\n";
    used_layers = lay_it->second.D;
    for (set<int>::iterator I=used_layers.begin(); I != used_layers.end(); I++) {
      for (int i = sheet_it->namex-LAYOUT.nav_right; 
               i <= sheet_it->namex+LAYOUT.nav_right; i++) {
        for (int j = sheet_it->namey-LAYOUT.nav_up; 
                 j <= sheet_it->namey+LAYOUT.nav_up; j++) {
          used_scraps.clear();
          map<string,set<string> >::iterator pr_it = 
            preview.find(xyz2str(*I,i,j));
          if (pr_it != preview.end()) used_scraps = pr_it->second;
          if (!used_scraps.empty()) {
            for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
              if (used_scraps.count(K->name) > 0 && 
                           NAV_SCRAPS.count(K->name) == 0 &&
                           K->I != "" && K->sect == 0) {
                xc = K->I1; yc = K->I2;
                xc -= LAYOUT.hsize * (sheet_it->namex - LAYOUT.nav_right) + 
                      LAYOUT.hoffset; 
                yc -= LAYOUT.vsize * (sheet_it->namey - LAYOUT.nav_up) + 
                      LAYOUT.voffset; 
                P << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                        tex_Xname("I"+(K->name)) << "}%" << endl;
                NAV_SCRAPS.insert(K->name);
              }
            }
          }
        }
      }
    }
  }
  NAV_SCRAPS.clear();
  P << "\\PL{0 g}%\n";
  for (int i = sheet_it->namex-LAYOUT.nav_right; 
           i <= sheet_it->namex+LAYOUT.nav_right; i++) {
    for (int j = sheet_it->namey-LAYOUT.nav_up; 
             j <= sheet_it->namey+LAYOUT.nav_up; j++) {
      used_scraps.clear();
      map<string,set<string> >::iterator pr_it = 
        preview.find(xyz2str(sheet_it->layer,i,j));
      if (pr_it != preview.end()) used_scraps = pr_it->second;
      if (!used_scraps.empty()) {
        for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
          if (used_scraps.count(K->name) > 0 && 
                       NAV_SCRAPS.count(K->name) == 0 &&
                       K->I != "" && K->sect == 0) {
            xc = K->I1; yc = K->I2;
            xc -= LAYOUT.hsize * (sheet_it->namex - LAYOUT.nav_right) + 
                  LAYOUT.hoffset; 
            yc -= LAYOUT.vsize * (sheet_it->namey - LAYOUT.nav_up) + 
                  LAYOUT.voffset; 
            P << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                    tex_Xname("I"+(K->name)) << "}%" << endl;
            NAV_SCRAPS.insert(K->name);
          }
        }
      }
    }
  }
  NAV_SCRAPS.clear();
  if (!lay_it->second.U.empty()) {
    P << "\\PL{0.2 w}%\n";
    used_layers = lay_it->second.U;
    for (set<int>::iterator I=used_layers.begin(); I != used_layers.end(); I++) {
      for (int i = sheet_it->namex-LAYOUT.nav_right; 
               i <= sheet_it->namex+LAYOUT.nav_right; i++) {
        for (int j = sheet_it->namey-LAYOUT.nav_up; 
                 j <= sheet_it->namey+LAYOUT.nav_up; j++) {
          used_scraps.clear();
          map<string,set<string> >::iterator pr_it = 
            preview.find(xyz2str(*I,i,j));
          if (pr_it != preview.end()) used_scraps = pr_it->second;
          if (!used_scraps.empty()) {
            for (list<scraprecord>::iterator K = SCRAPLIST.begin(); K != SCRAPLIST.end(); K++) {
              if (used_scraps.count(K->name) > 0 && 
                           NAV_SCRAPS.count(K->name) == 0 &&
                           K->B != "" && K->sect == 0) {
                xc = K->B1; yc = K->B2;
                xc -= LAYOUT.hsize * (sheet_it->namex - LAYOUT.nav_right) + 
                      LAYOUT.hoffset; 
                yc -= LAYOUT.vsize * (sheet_it->namey - LAYOUT.nav_up) + 
                      LAYOUT.voffset; 
                P << "\\PB{" << xc << "}{" << yc << "}{\\" << 
                        tex_Xname("B"+(K->name)) << "}%" << endl;
                NAV_SCRAPS.insert(K->name);
              }
            }
          }
        }
      }
    }
  }

  // navigator grid
  P << "\\PL{Q}" << "\\PL{0 0 " << HSN << " " << VSN << " re S 0.1 w}";
  for (int i = 1; i < nav_x; i++)      
    P << "\\PL{" << HSN*i/nav_x << " 0 m " << HSN*i/nav_x << " " << VSN << " l S}%\n";
  for (int i = 1; i < nav_y; i++)
    P << "\\PL{0 " << VSN*i/nav_y << " m " << HSN << " " << VSN*i/nav_y << " l S}%\n";
  P << "\\PL{0.4 w " <<
    HSN*LAYOUT.nav_right/nav_x << " " << VSN*LAYOUT.nav_up/nav_y << " " <<
    HSN/nav_x << " " << VSN/nav_y << " " << " re S}";
  // XObject definition
  P << "\\hfill}\\ht\\xxx=" << VSN << "bp\\dp\\xxx=0bp\n";
  P << "\\immediate\\pdfxform\\xxx\\newcount\\" << 
       tex_Nname(u2str(sheet_it->id)) << " \\" <<
       tex_Nname(u2str(sheet_it->id)) << "=\\pdflastxform" << endl;
}

void print_grid(ofstream& PAGEDEF) {
  PAGEDEF << "\\PL{q}";
  PAGEDEF << "\\PL{3 w 0 0 " << HS << " " << VS << " re S}";
  double i = LAYOUT.hsize + LAYOUT.overlap; 
  double j = LAYOUT.vsize + LAYOUT.overlap;
  PAGEDEF << "\\PL{0.5 w}";
  PAGEDEF << "\\PL{0 " << LAYOUT.overlap << " m " << HS << " " << 
                          LAYOUT.overlap << " l S}";
  PAGEDEF << "\\PL{0 " << j << " m " << HS << " " << j << " l S}";
  PAGEDEF << "\\PL{" << LAYOUT.overlap << " 0 m " << LAYOUT.overlap << 
             " " << VS << " l S}";
  PAGEDEF << "\\PL{" << i << " 0 m " << i << " " << VS << " l S}";
// add actual grid here  
  PAGEDEF << "\\PL{Q}%" << endl;
}


void build_pages() {
  
  ofstream PAGEDEF("th_pagedef.tex");
  if(!PAGEDEF) therror(("Can't write file"));
  PAGEDEF.setf(ios::fixed, ios::floatfield);
  PAGEDEF.precision(1);

  ofstream PAGE("th_pages.tex");
  if(!PAGE) therror(("Can't write file"));
  PAGE.setf(ios::fixed, ios::floatfield);
  PAGE.precision(1);

  ofstream PDFRES("th_resources.tex");
  if(!PDFRES) therror(("Can't write file"));
  if (LAYOUT.transparency || LAYOUT.OCG) {
    PDFRES << "\\ifnum\\pdftexversion<110\\pdfcatalog{ /Version /" <<
      (LAYOUT.OCG ? "1.5" : "1.4") << " }" << 
      (LAYOUT.OCG ? "\\else\\pdfoptionpdfminorversion=5" : "") << "\\fi" << endl;
  }

  if (LAYOUT.transparency) {
    PDFRES << "\\immediate\\pdfobj{ << /GS0 " <<
                 "<< /Type /ExtGState /ca 1 /BM /Normal >> " <<
           " /GS1 << /Type /ExtGState /ca \\the\\opacity\\space /BM /Normal >> >> }" << endl;
    PDFRES << "\\newcount\\resid\\resid=\\pdflastobj" << endl;
    PDFRES << "\\immediate\\pdfobj{ << /S /Transparency /K true >> }" << endl;
    PDFRES << "\\newcount\\attrid\\attrid=\\pdflastobj" << endl;
  }
  else {
    PDFRES << "\\immediate\\pdfobj{ << /GS0 " <<
                 "<< /Type /ExtGState >> " <<
           " /GS1 << /Type /ExtGState >> >> }" << endl;
    PDFRES << "\\newcount\\resid\\resid=\\pdflastobj" << endl;
  }

  if (LAYOUT.OCG) {
    PDFRES << "\\immediate\\pdfobj{ << /Type /OCG /Name ([Preview above]) >> }" << endl;
    PDFRES << "\\newcount\\ocU\\ocU=\\pdflastobj" << endl;
    PDFRES << "\\immediate\\pdfobj{ << /Type /OCG /Name ([Preview below]) >> }" << endl;
    PDFRES << "\\newcount\\ocD\\ocD=\\pdflastobj" << endl;
    if (mode == MAP) {
      for (map<int,layerrecord>::iterator I = LAYERHASH.begin();
                                          I != LAYERHASH.end(); I++) {
        if (I->second.Z == 0) {
          PDFRES << "\\immediate\\pdfobj{ << /Type /OCG /Name <feff" <<
            utf2texhex(I->second.N) << "> >> }" << endl;
          PDFRES << "\\newcount\\oc" << u2str(I->first) << "\\oc" << 
                     u2str(I->first) << "=\\pdflastobj" << endl;
        }
      }
    }
    PDFRES << "\\pdfcatalog{ /OCProperties <<" << endl <<
              "  /OCGs [\\the\\ocU\\space0 R ";
    if (mode == MAP) {
      for (map<int,layerrecord>::iterator I = LAYERHASH.begin();
                                          I != LAYERHASH.end(); I++) {
        if (I->second.Z == 0) 
          PDFRES << "\\the\\oc" << u2str(I->first) << "\\space 0 R ";
      }
    }
    PDFRES << "\\the\\ocD\\space0 R]" << endl <<
              "  /D << /Name (Map layers) /ListMode /VisiblePages" << 
                     " /Order [\\the\\ocU\\space0 R ";
    if (mode == MAP) {
      for (map<int,layerrecord>::reverse_iterator I = LAYERHASH.rbegin();
                                          I != LAYERHASH.rend(); I++) {
        if (I->second.Z == 0) 
          PDFRES << "\\the\\oc" << u2str(I->first) << "\\space 0 R ";
      }
    }
    PDFRES << "\\the\\ocD\\space0 R] >>" << endl << ">> }" << endl;
  }

  if (LAYOUT.doc_author != "") 
    PDFRES << "\\author{" << utf2texhex(LAYOUT.doc_author) << "}" << endl;
  if (LAYOUT.doc_subject != "") 
    PDFRES << "\\subject{" << utf2texhex(LAYOUT.doc_author) << "}" << endl;
  if (LAYOUT.doc_keywords != "") 
    PDFRES << "\\keywords{" << utf2texhex(LAYOUT.doc_author) << "}" << endl;
  if (LAYOUT.doc_title != "") 
    PDFRES << "\\title{" << utf2texhex(LAYOUT.doc_author) << "}" << endl;

  PDFRES << "\\pdfcatalog { /TeXsetup /" << pdf_info() << " }" << endl;

  PDFRES.close();

  if (mode == ATLAS) {
    HS = LAYOUT.hsize + 2*LAYOUT.overlap;
    VS = LAYOUT.vsize + 2*LAYOUT.overlap;
    if (LAYOUT.page_numbering) {
      PAGE << "\\pagenumberingtrue" << endl;
    }
  }
  else {
    if (LAYOUT.map_grid) {
      MINX = LAYOUT.hsize * floor (MINX/LAYOUT.hsize);
      MINY = LAYOUT.vsize * floor (MINY/LAYOUT.vsize);
      MAXX = LAYOUT.hsize * ceil (MAXX/LAYOUT.hsize);
      MAXY = LAYOUT.vsize * ceil (MAXY/LAYOUT.vsize);
    }
    HS = MAXX - MINX;
    VS = MAXY - MINY;
    if (HS>14000 || VS>14000) 
      therror(("Map is too large for PDF format. Try smaller scale!"));
    PAGEDEF << "\\eject" << endl;
    PAGEDEF << "\\hsize=" << HS << "bp" << endl;
    PAGEDEF << "\\vsize=" << VS << "bp" << endl;
    PAGEDEF << "\\pdfpagewidth=\\hsize\\advance\\pdfpagewidth by " <<
                                             2*LAYOUT.overlap << "bp" << endl;
    PAGEDEF << "\\pdfpageheight=\\vsize\\advance\\pdfpageheight by " <<
                                             2*LAYOUT.overlap << "bp" << endl;
    PAGEDEF << "\\hoffset=0cm" << endl;
    PAGEDEF << "\\voffset=0cm" << endl;
    PAGEDEF << "\\pdfhorigin=" << LAYOUT.overlap << "bp" << endl;
    PAGEDEF << "\\pdfvorigin=" << LAYOUT.overlap << "bp" << endl;
  }
  
  if (mode == ATLAS) {
    for (list<sheetrecord>::iterator I = SHEET.begin(); 
                                     I != SHEET.end(); I++) {

//      cout << "ID: " << I->id << " Layer: " << I->layer << " X: " << 
//           I->namex << " Y: " << I->namey << endl;

// cout << "*" << flush; 

      PAGEDEF << "\\setbox\\xxx=\\hbox to "<< HS << "bp{%" << endl;

      print_map(I->layer, PAGEDEF, I);
      print_grid(PAGEDEF);

      PAGEDEF << "\\hfill}\\ht\\xxx=" << VS << "bp\\dp\\xxx=0bp" << endl;
      PAGEDEF << "\\immediate\\pdfxform";
      PAGEDEF << "\\xxx\\newcount\\" << tex_Sname(u2str(I->id)) <<
             " \\" << tex_Sname(u2str(I->id)) << "=\\pdflastxform" << endl;

      print_navigator(PAGEDEF,I);
      compose_page(I, PAGE);

    }
  }
  else {
    PAGEDEF << "\\setbox\\xxx=\\hbox to " << HS << "bp{%" << endl;
    if (!MAP_PREVIEW_DOWN.empty()) print_preview(0,PAGEDEF,MINX,MINY,NULL);
    for (map<int,layerrecord>::iterator I = LAYERHASH.begin();
                                        I != LAYERHASH.end(); I++) {
      if (I->second.Z == 0) {
        PAGEDEF << "\\setbox\\xxx=\\hbox to " << HS << "bp{%" << endl;
                            // we need flush layer data using XObject 
                            // (the text clipping path may become too large)

        print_map((*I).first,PAGEDEF,NULL);

        PAGEDEF << "\\hfill}\\ht\\xxx=" << VS << "bp\\dp\\xxx=0bp" << endl;
        PAGEDEF << "\\immediate\\pdfxform ";
        if (LAYOUT.OCG) {
          PAGEDEF << "attr{/OC \\the\\oc" << u2str(I->first) << "\\space 0 R} ";
        }
        PAGEDEF << "\\xxx\\PB{0}{0}{\\pdflastxform}%" << endl;
      }
    }
    if (!MAP_PREVIEW_UP.empty()) print_preview(1,PAGEDEF,MINX,MINY,NULL);
    if (LAYOUT.map_grid) {
      PAGEDEF << "\\PL{q .4 w}%" << endl;
      PAGEDEF << "\\PL{0 0 " << HS << " " << VS << " re S}%" << endl;
      for (double i=0; i <= HS; i += LAYOUT.hsize) {
        PAGEDEF << "\\PL{" << i << " 0 m " << i << " " << VS << " l S}%" << endl;
      }
      for (double i=0; i <= VS; i += LAYOUT.vsize) {
        PAGEDEF << "\\PL{0 " << i << " m " << HS << " " << i << " l S}%" << endl;
      }
      PAGEDEF << "\\PL{Q}%" << endl;

    }
    PAGEDEF << "\\setbox\\xxx=\\hbox{";                     // map legend
    PAGEDEF << "\\vbox to " << VS << "bp{\\maplegend\\vfill}}" << endl;
    PAGEDEF << "\\pdfxform\\xxx\\PB{0}{0}{\\pdflastxform}" << endl;
    PAGEDEF << "\\hfill}\\ht\\xxx=" << VS << "bp\\dp\\xxx=0bp" << endl;
    PAGEDEF << "\\pdfxform\\xxx\\PB{0}{0}{\\pdflastxform}%" << endl;
  }

  PAGEDEF.close();
  PAGE.close();
}


int thpdf(int m) {
  mode = m;

#ifdef NOTHERION
  init_encodings();
  print_fonts_setup();
  cout << "making " << ((mode == ATLAS) ? "atlas" : "map") << " ... " << flush;
#else
  thprintf("making %s ... ", (mode == ATLAS) ? "atlas" : "map");
#endif

#ifdef NOTHERION
  read_settings();   // change to the quick mode only
#endif    

  make_sheets();
  if (mode == ATLAS) {
    sort_sheets();
    find_jumps();
  }
  build_pages();

#ifdef NOTHERION
  cout << "done" << endl;
#else
  thprintf("done\n");
#endif
  return(0);
}

#ifdef NOTHERION
int main() {
  thpdf(0);
}
#endif

