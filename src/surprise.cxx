/*
  $Id: decomp.cxx 15344 2012-10-29 13:41:03Z sloot $
  $URL: https://ilk.uvt.nl/svn/sources/tscan/trunk/src/decomp.cxx $

  Copyright (c) 1998 - 2012
 
  This file is part of tscan

  tscan is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  tscan is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      
  or send mail to:
      
*/

#include <cstdio> // for remove()
#include <cstring> // for strerror()
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <iostream>
#include <fstream>
#include "config.h"
#include "libfolia/folia.h"
#include "libfolia/document.h"
#include "ticcutils/PrettyPrint.h"
#include "tscan/surprise.h"

using namespace std;
using namespace folia;
using namespace TiCC;

string translatePos( const Word *w ){
  vector<PosAnnotation*> posV = w->select<PosAnnotation>("http://ilk.uvt.nl/folia/sets/frog-mbpos-cgn");
  if ( posV.size() != 1 )
    throw ValueError( "word doesn't have Frog POS tag info" );
  string head = posV[0]->feat("head");
  if ( head == "N" )
    return "N";
  else if ( head == "LID" )
    return "Art";
  else if ( head == "WW" )
    return "V";
  else if ( head == "VNW" )
    return "Pron";
  else if ( head == "VG" )
    return "Conj";
  else if ( head == "LET" )
    return "Punc";
  else if ( head == "BW" )
    return "Adv";
  else if ( head == "ADJ" )
    return "Adj";
  else if ( head == "TSW" )
    return "Int";
  else if ( head == "TW" )
    return "Num";
  else if ( head == "VZ" )
    return "Prep";
  else if ( head == "SPEC" ){
    string pos = w->pos();
    if ( pos == "SPEC(deeleigen)" ) 
      return "N";
    else if ( pos == "SPEC(symb)" ) 
      return "Conj";
    else if ( pos == "SPEC(enof)" ) 
      return "Conj";
    else if ( pos == "SPEC(vreemd)" ) 
      return "N";
    else if ( pos == "SPEC(afgebr)" ) 
      return "N";
    else if ( pos == "SPEC(afk)" ) 
      return "N";
    else
      return "Misc";
  }
  else
    return head;
}

vector<double> runSurprisal( Sentence* sent, const string& path ){
  struct stat sbuf;
  int res = stat( "/tmp/surprise", &sbuf );
  if ( !S_ISDIR(sbuf.st_mode) ){
    res = mkdir( "/tmp/surprise/", S_IRWXU|S_IRWXG );
    if ( res ){
      cerr << "problem: " << res << endl;
      exit( EXIT_FAILURE );
    }
  }
  ofstream os( "/tmp/surprise/surprise.in" );
  vector<Word*> words = sent->words();
  for ( size_t i=0; i < words.size(); ++i ){
    os << words[i]->text () << "\t" << translatePos( words[i] ) << endl;
  }
  os.close();
  string cmd = path + "surprise.sh /tmp/surprise/surprise.in /tmp/surprise/surprise.out";
  res = system( cmd.c_str() );
  if ( res ){
    cerr << "RES = " << res << endl;
    cerr << "failed command: " << cmd << endl;
  }
  remove( "/tmp/surprise/surpise.in" );
  ifstream is( "/tmp/surprise/surprise.out" );
  vector<double> results;
  while( is ){
    string line;
    getline( is, line );
    if ( line.empty() || line.find( "***" ) != string::npos )
      continue;
    vector<string> v;
    int n = split_at( line, v, "\t" );
    if ( n == 4 ){
      double surp = stringTo<double>( v[3] );
      results.push_back( surp );
    }
  }
  return results;
}

