IEDA Final Project - CAD Contest Problem B

[Members]
電機三 鄭定緯, Ding-Wei Cheng, B09901021
電機三 林奕廷, Yi-Ting Lin   , B09901068
電機三 陳立強, Li-Chiang Chen, B09901097

[Folders]
results: 目前的實驗結果，檔名內有對應不同的case，每個檔前面的數字為使用大會提供的evaluator所測得的score。 

[Program Files - ICCAD_B.cpp]
根據提示，輸入相關檔名和MODE使用相關功能。
----------------------------------------------------------------------------
測資讀入: 輸入ProblemB_caseN，不用加.txt !
MODE選擇: 根據提示選擇想要使用的placement方式或生成output.txt。
----------------------------------------------------------------------------
MODE類型：(詳細請對照報告書的placement section閱讀使用)
partition_generateTOP&BOT：進行initilization&partition，生成擺TOP和BOT所需要的NTUplace3檔案。

all_pinprojection_generateBOT：進行All Pins Projection，讀進TOP的placement之.ntup.pl檔案，產生擺BOT所需要的NTUplace3檔案。

single_pinprojection_generateBOT：進行Single Proxy Projection，讀進TOP的placement之.ntup.pl檔案，產生擺BOT所需要的NTUplace3檔案。

terminal_afterplaceTOP_generateBOT：進行Terminal-Based Bottom-Die Placement，讀進TOP的placement之.ntup.pl檔案，產生擺BOT所需要的NTUplace3檔案。

terminal_afterplaceTOPBOT_RESULT：讀進TOP和BOT的.ntup.pl檔案們，再來擺放terminal，並產生output.txt檔。

terminal_afterplaceTOP_RESULT：獲得Terminal-Based Bottom-Die Placement的結果。

terminal_lookup2times_RESULT：進行Terminal Replace，讀進TOP和BOT(記得這個是先進行過Terminal-Based Bottom-Die Placement的結果的檔案)的.ntup.pl檔案們，再次擺放terminal，並產生output.txt檔。
----------------------------------------------------------------------------
MODE若是結尾沒有_RESULT，意即有generate，則會產生NTUplace3所需要的檔案們(.aux, .nodes, .nets, .wts, .pl, and .scl)，產生這些檔案後請到Linux系統中創建資料夾，將ntuplace3和這六個檔案放在同個資料夾中，便可使用ntuplace3的指令進行擺設。
進行擺設後，會生成.ntup.pl檔案，即為placement結果，再將此檔案拖回ICCAD_B.cpp所在的資料夾，即可以讀到placement的結果(檔名記得要打對，產生出來的檔名有點長)。

[NTUplace3指令]
./ntuplace3 -aux (要擺的檔案).aux -ulit (面積利用率) -MRT
會生成.gb.pl, .lg.pl, .ntup.pl等檔案，記得只要使用.ntup.pl檔案就好。
-MRT為令Macro可以旋轉。

[Other Files]
ProblemB_case1.txt, ProblemB_case4.txt, ProblemB_case3.txt, ProblemB_case4.txt: 大會提供測資

[備註]
Case3 & Case4目前都只有產生合法解，意即只能進行Basic Placement for Valid Solution的部分，其他的部分runtime應該會爆炸，這部分可以參見我們報告書中的未來展望section。
