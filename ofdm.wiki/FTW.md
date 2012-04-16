## FTW IEEE802.11a/g/p OFDM Frame Encoder
Website: https://www.cgran.org/wiki/ftw80211ofdmtx           
Paper: http://userver.ftw.at/~zemen/papers/Fuxjaeger10-WSR-paper.pdf

## 802.11a Design
### PHY
[[ppduframe.gif]]

Note: FCS 4 Bytes, and 位置在Tails前面

### MAC
[[ofdmpaystruct.gif]]

Note: FCS 4 Bytes, and 位置在Tails前面


## FTW Flow-Graph
* port 0: self._pkt_input -> self.pilot -> self.cmap -> self.ifft -> self.cp_adder -> self.scale -> self.s2v -> self.preamble -> self.zerogap -> self.repeat -> self.v2s -> self.amp -> self.u
* self.cmap  = ftw.ofdm_cmap_cc(self._fft_length, self._total_sub_carriers)
* port 1:  (self._pkt_input, 1) -> (self.preamble, 1) -> (self.zerogap, 1)
* 生成相应的data -> 添加signal and service field -> scrambler (注意代码中没有对signal field做此操作) -> conv_encoder (先按照1/2搞，然后punture) -> inteleaver (如果不是BPSK modulation, 前48bits - signal field单独处理) -> insert into queue进行modulation (实现见ftw_ofdm_mapper, 其中constellation_bpsk and FIRST_SYMBOL就是专门处理signal field)

###Scrambler代码
* for k in range (24 , len(app)): 前面0~23是singal字段不能scramble
* Scramble: data字段（包括service）与定义好的seq字段XOR，seq字段长度127，从signal字段后开始对齐(k-24)
* 最后加上6个没有scramble的TAIL bits: 6 zeros. for i in range (23 + zero_forcing_index +17 , 23 + zero_forcing_index + 22 + 1): 其中23是signal字段，17是services字段，因为zero_forcing_index没有包括services

###Conv代码
* The PPDU tail bit field is six bits of "0", which are required to return the convolutional encoder to the "zero state."
* 下面的图可以完全说明其过程：

[[conv.png| width=800px]]

###Interleaver
按照标准定义理解。只会在1个Symbol内部Inteleave.

###不同包生成的I/Q Samples的差异
如果只是mac ctl每次+1, 同时FCS也改变了：

* scrambler相邻包只影响1个byte（实际上序号占2个byte），然后FCS 4Bytes都不同;
* conv的结果（1/2），seqctl位置2个byte，最多可能影响6Byte，因为conv是1/2，不同的2B的seqctl有4B不同，加上 conv_code有个register，里面状态会继续影响2Byte. 同理FCS最多影响10Byte。根据scramble后的相似程度，可能会低于6 Byte and 10 Byte.
* interleaver: 每个symbol里面变化，比如BPSK and 1/2，就是6 Byte一起变化，所以conv不同的地方可能还会扩散到隔壁的Byte，以满足3 Byte的Block结构。

###包参数
* SINGAL: 24bits = 3Bytes, 单独map到一个symbol, 所以一般单独计算
* SERVICE：全0， 2 Bytes
* MAC Header: 2 + 2 + 6 + 6 + 6 + 2 (SeqCtrl) + 2 = 26Bytes, 不包括Address 4
* LLC: 8 Bytes
* IP Header: 10 + 2 (checksum) + 4 (sour) + 4 (dest) = 20 Bytes
* UDP Header: 2 (src) + 2 (dest) + 2 (length) + 2 (checksum) = 8 Bytes
* Payload: 如果是HelloWorld, 10Bytes
* FCS: 4 Bytes
* TAIL_and_PADING: 6bits zero + variable, 如果是HelloWorld, 3Bytes

