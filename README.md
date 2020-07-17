# README.md about ydk2txt

遊戯王OCGシミュレーターであるADSにおいて、デッキ内容の保存ファイルとして使用されている`.ydk`ファイルから日本語名のリストを作成します。
なお、**ADSは日本語化されている必要はありません**。

## How to use

1. ADSのカードデータベースなどを利用するため、**本ディレクトリ`YGO_ydk2txt`はADSのディレクトリ内に置きます**。(deck, replay, expansionsなどと同じか、それより深い階層)
2. 日本語名のカードリストテキストに変換したい`.ydk`ファイルを`YGO_ydk2txt/deck/`内にコピーします。
3. OSに合わせて実行ファイルをダブルクリック、またはターミナルから実行します。
    - Windowsの場合は`YGO_ydk2txt/YGO_ydk2txt.exe`
    - Linux (and Mac?) の場合は`YGO_ydk2txt/YGO_ydk2txt.out`

## If you want to use from script

インターネットからダウンロードした実行ファイルをそのまま実行するのは危険と考えるのは当然なので、sourceファイルも公開します。
`YGO_ydk2txt/script/`内にあるので、都合の良い言語のファイルを利用してください。機能は基本的に互換です。

|ファイル名|言語|対応OS|
|-|-|-|
|YGO_ydk2txt_windows.cpp|C++|Windows|
|YGO_ydk2txt_linux.cpp|C++|Linux|
|YGO_ydk2txt.py|Python|Linux|

## 仕組み

`.ydk`ファイルは単なるテキストであり、`#main`や`#extra`などの区切りおよび`14920218`などの各カード固有の通し番号(id)が記載されている。(これは実際のカードの隅に記載されているものと同じ。)
カードデータベースではなく、`ADS_PATH/expansions/official/c14920217.lua`などのscriptファイル内の冒頭に日本語名が記載されている。
そこで`.ydk`ファイルを読み込み、カードidのからそのscriptファイルを参照して、日本語名を取得した。

## 発展

カードデータベースやカード画像から、さまざまな情報をカードidで参照することができる。
詳しい話は**効率よく詳細初手確率計算**の話のときにでも。

## 開発環境

|言語|version|Comment|
|-|-|-|
|C++|g++ 9.2.0, MinGW, Windows|Compileには引数として`-std=c++17`が必要|
|C++|g++ 7.5.0, WSL(Windows10)||
|Python|Python3.6.9, WSL(Windows10)| jupyter labでより幅広い機能を実装した中から抽出し、調整した|

ADSはProjectIgnis(38.1.2)をもとにしたが、他のADSでも大きくは変わらないと**思われる**。