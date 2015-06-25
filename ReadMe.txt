
	      Win32/UNIX/Human68K バイナリテキストコンバータ
		      ish/uuencode 対応 aish v1.13


  ish/uuencode などの、バイナリとテキストを相互変換するプログラムです。実に
1995年2月以来、3年ぶりの修正です。

  前回(といっても 1995年)の aish v1.12 では既に、Base64 を展開できるオプショ
ン -m がついていました。このオプションを使うと、メールなどに添付されたバイナ
リも展開して取り出すことができます。ただ当時は MIME 以前にインターネット自体
普及しておらず、対応したものの使う人もいなければ動作チェックをする機会もほと
んどなく、結局ほとんどテストしないままの実装になっていました。

  それが昨今のインターネット普及で、X680x0 でも MIME が使われるようになり、
 aish の展開機能も ようやく 必要とされるようになりました。データ内の出力ファ
イル名に対応してほしいとのことなので、その修正を加えたのがこの aish v1.13 で
す。それ以外の機能は v1.12 と全く同じです。


●配布パッケージ

	aish113s.lzh	ソース配布キット。UNIX版含む。他の環境でもコンパイル
			可能。
	aish113x.lzh	Human68k 用バイナリ配布キット
	aish113w.lzh	Win32 用バイナリ配布キット


●付属ファイル

・aish113s.lzh

	ReadMe.txt	このテキスト

	install.txt	UNIX 向けソースキットの導入テキスト。X680x0 でのコン
			パイル方法も解説。

	aish.txt	マニュアル

	ソースリスト一式


・aish113x.lzh

	ReadMe.txt	このテキスト

	aish.txt	マニュアル

	aish.x		aishコマンド
	aishmv.x	aishmvコマンド


・aish113w.lzh

	ReadMe.txt	このテキスト

	aish.txt	マニュアル

	aish.exe	aishコマンド
	aishmv.exe	aishmvコマンド


1998/3/19 小笠原博之
oga@art.udn.ne.jp

