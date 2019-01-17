anap4  Ver. 0.92 June 5, 2014  KAWABATA Takahiro

anap4 は課題研究P4においてNBBQで取得したデータを解析するためのプログラ
ムです。

anap4_X.XX.tar.gz を取得したら、適当な場所にコピーして解凍します。

> tar xvfz anap4_X.XX.tar.gz

(X.XX は適切なバージョン番号に置き換えます。)

解凍後は、anap4 のディレクトリに入って、userdef.h histdef.c と anaevt.c
を実験条件に合わせて編集し、make してコンパイルします。

> cd anap4
> emacs userdef.h
> emacs histdef.c
> emacs anaevt.c
> make

◯使い方
> ./anap4 -online -g [glob_sect] -hb [hbfile] infile1 [infile2] ....

-online: オンライン解析のときに指定します。NBBQ が取得したデータをサン
         プリングして解析します。
         ・DAQ が動いているマシンの clihost.txtに解析マシンが指定され
           ている。
         ・解析マシンで analyzer が起動している。
         必要があります。
-g: PAW のグローバルセクションを用いて解析します。[glob_sect] にはグロー
    バルセクションの名前を定義します。名前には大文字を用いる必要があり
    ます。[glob_sect] を定義しない場合には、グローバルセクションの名前
    は PAPA になります。
-hb [hbfile]: 結果を出力するファイル名を hbfile を指定しないときの出力
              ファイル名は test.hb となります。
infile1, [infile2] ...: 入力ファイル名を指定します。複数のファイルが指
                        定された場合には、複数のファイルを結合して解析
                        します。

anap4 による解析結果は PAW を用いて表示させます。PAW の使い方について
は ggrks。

◯userdef.h の編集
userdef.h では、ユーザが自分で定義したいマクロを宣言します。ここで
宣言したマクロは、後述する histdef.c や anaevt.c の中で用いることがで
きます。
---- userdef.h の例 -----------------
/* Macro defined by users */

#define N_QDC 2
---- ここまで -----------------


◯histdef.c の編集
histdef.c では解析に用いるヒストグラムを定義します。
---- histdef.c の例 -----------------

---- ここまで -----------------
1次元のヒストグラムを定義する。
HBOOK1(ID,HNAME,NX,XMIN,XMAX,VMX)
ID ... ヒストグラムのID。他のヒストグラムと重複しないように整数で定義
       する。
HNAME ... ヒストグラムの名前。文字列で定義する。
NX ... ヒストグラムのチャンネルの数。整数。
XMIN ... ヒストグラムの下限値。整数。
XMAX ... ヒストグラムの上限値。整数。
VMX ... 1チャンネル当たりのカウント数の上限。通常は0を指定すればよい。

2次元のヒストグラムを定義する。
HBOOK2(ID,HNAME,NX,XMIN,XMAX,NY,YMIN,YMAX,VMX)


◯anaevt.c の編集
anaevt.c では、取得したデータの1イベント毎に解析します。
イベントのデータは配列 rawbuf として与えられます。
イベントのデータ長は evtlen から取得します。
このルーチンでは、rawbuf[0] --- rawbuf[evtlen-1] に格納されているデー
タを取り出し、これを解析してヒストグラムに入力します。データの並び順は、
NBBQ の evt.c で定義した順になります。詳しいデータフォーマットについて
は、

http://rarfaxp.riken.go.jp/~baba/acquisition/system/nbbq/index.html
http://rarfaxp.riken.go.jp/~baba/acquisition/system/dataformat.html

あたりを参照してください。

解析に用いるヒストグラムは histdef.c の中で定義されている必要がありま
す。

解析に必要な変数は、関数 anaevt() の中のローカル変数として定義します。
解析を始める前に変数を初期化するのを忘れないようにしましょう。

エネルギー較正のための定数(下の例で言うところの qpar など)は、const 
を指定して、定数として宣言しておくとよいでしょう。

anaevt.c の例 -----------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cfortran.h>
#include <hbook.h>
#include <kernlib.h>

#include "kinema.h"
#include "anap4.h"

int anaevt(int evtlen,unsigned short *rawbuf,struct p4dat *dat){
  int i,ip;
  int segsize,ids,ips,ipsn;
  unsigned short qdc[N_QDC];
  double qdcc[N_QDC];
  const double qpar[N_QDC][2]={{0.0, 0.01}, {0.0, 0.01}};

  /************* Clear Event Buffer *****************/
  for(i=0;i<N_QDC;i++) {
    qdc[i]=0;
    qdcc[i]=0;
  }
  /************* Clear Event Buffer *****************/

  /************* Decode Event Data Here *****************/
  ip=0;
  while(ip<evtlen){  
    int tmpdat,tmpch;

    /** Segment Header ************/
    segsize=rawbuf[ip++];
    ipsn=ip+segsize-1;
    ids=rawbuf[ip++];

    while(ip<ipsn){ /*** Segment loop ***/
      /*** 7166 ****/
      for(i=0;i<N_QDC;i++) {
	tmpdat=rawbuf[ip++];
	tmpch=(tmpdat>>12)&0xf;
	qdc[i]=(tmpdat&0xfff);
      }
    }
  }
  /**** Decode Event Data Above***************/

  /**** Data Analysis Here ***************/
  /* energy calibration */
  for(i=0;i<N_QDC;i++) {
    qdcc[i]=(qdc[i]-qpar[i][0])*qpar[i][1];
  }
  /**** Data Analysis Above ***************/

  /*********** Booking here **********/
  for(i=0;i<N_QDC;i++){
    HF1(10+i,qdc[i],1.0);
    HF1(12+i,qdcc[i],1.0);
  }
  HF2(100,qdc[0],qdc[1],1.0);
  /*********** Booking Above **********/
  return(ip);
}

---- ここまで -----------------
ヒストグラムにデータを入力する
HF1(ID,XVAL,WEIGHT)
HF2(ID,XVAL,YVAL,WEIGHT)

ID ... ヒストグラムのID。前もって histdef.c の中で定義されている必要が
       ある。
XVAL, YVAL ... ヒストグラムに入力するデータ。
WEIGHT ... ヒストグラムにデータを入力する際の重み。通常は 1.0 を使う。
