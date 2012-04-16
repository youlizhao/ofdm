## RawOFDM by MIT
基于Benchmark OFDM，几点不同:
* 添加了Pilot
* 添加了Channel Coding, Interleavor, Scrambler等
* 简化了插入数据部分


关于benchmark的pilot: While I wrote the transmitter to allow for pilot tones, we don't actually make use of them in the receiver. Instead, we use a decision feedback equalizer to keep on track. Making use of the pilot tones in the receiver is definitely something we need done, though.（http://old.nabble.com/high-level-OFDM-question-td27567642.html）


关于RawOFDM的pilot: 采用的是cur_pilot每插一个symbol，变换方向（1 -> -1 -> 1 -> ...），不是标准规定的。只要receiver相应的作变化就可以了。 代码里面有这个说明：
      // TODO: FIXME: 802.11a-1999 p.23 (25) defines p_{0..126v} which is cyclic // NOTE: but there are at most 20 symbols in our frames, so at most 80 in the seq are used
      // Fill in pilots

###代码分析
RawOFDM可以分成两个部分分析，with or without channel coding. 先分析without channel coding
(1) self.u -> ofdm_rxtx.RX()
(2) ofdm_rxtx.RX():  ->  raw.ofdm_demod (2 port out) -> raw.fix_frame (2 port in) ->
(3) raw.ofdm_demod: -> ofdm_receiver (2 port in, 1 port out) -> raw.ofdm_demapper (2 port in, 2 port out w.p.) ->
(4) ofdm_receiver: 找到beginning, 做完corase freq. compensation
(5) raw_ofdm_demapper: 根据pilot进一步demap (output: still complex)
