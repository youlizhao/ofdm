% This script generates one OFDM frame in complex digital baseband
% representation according to the encoding procedure given by the
% 802.11-2007 standard.
%
% Contributors: Paul Fuxjaeger
%               Paolo Castiglione 
%               Pavle Belanovic
%
% Forschungszentrum Telekommunikation Wien 
% http://www.ftw.at 
% Vienna 2009
%
%
% Important Notes:
%
% First
% -----
% If sent by the USRP2 in the 2.4GHz ISM band using an interpolation
% factor of 5 a 802.11g capable receiver should be able to decode. If sent
% in the 5.2-5.8GHz band you get 802.11a.
% 
% If sent at 5.9GHz with an interpolation factor of 10 you get the 802.11p 
% draft (for vehicular communication applications).
% 
% Second
% ------
% MAC headers are already there, the IP layer is totally absent. 
% You may need to put your chipset into monitor/promiscuous mode 
% in order to see this frame at the MAC layer of the receiver stack. CRC32
% is automatically calculated by this script based on the whole MPDU.
%
% Third
% -----
% Use the transmit.py script to send. This takes the matlabframe.dat
% file and forwards it to the USRP2 sink.

clear all;

% User defined part starts here
%------------------------------
mac_body      = '';              % put your desired MSDU payload here
symbolduration= 4;               % OFDM symboltime in microseconds (4=11a/g, 8=11p)
regime        = 8;               % for 802.11a/g this means 36Mbit/s mode
mac_framectrl = '0800';          % this is a data frame
mac_duration  = '0000';          % is calculated further down
mac_address1  = '006008cd37a6';  % destination mac-address
mac_address2  = '0020d6013cf1';  % source mac-address
mac_address3  = '006008ad3baf';  % BSSID mac-address
mac_seqctrl   = '0000';          % 0 sequence and 0 fragment number
mac_crc       = '00000000';      % is calculated further down
%------------------------------
% User defined part stops here


% % optional: MSDU of reference frame in 802.11-2007.pdf Annex G
% 
% symbolduration= 4;               % in microseconds (4=11a/g, 8=11p)
% regime        = 6;               % for 802.11a/g this means 36Mbit/s mode
% mac_framectrl = '0402';          % this is a data frame
% mac_duration  = '0000';          % is calculated further down
% mac_address1  = '006008cd37a6';  % destination mac-address
% mac_address2  = '0020d6013cf1';  % source mac-address
% mac_address3  = '006008ad3baf';  % BSSID mac-address
% mac_seqctrl   = '0000';          % 0 sequence and 0 fragment number
% mac_body      = '4a6f792c2062726967687420737061726b206f6620646976696e6974792c0a4461756768746572206f6620456c797369756d2c0a466972652d696e73697265642077652074726561';
% mac_crc       = '00000000';      % is calculated further down

% % optional fun/hackerish stuff: disassociate a remote STA from the AP using
% % this MPDU:
%
% symbolduration= 4;               % in microseconds (4=11a/g, 8=11p)
% regime        = 1;               % for 802.11a/g this means 1Mbit/s mode
% mac_framectrl = '9000';          % MGMT disassociation frame
% mac_duration  = '0000';          % is calculated further down
% mac_address1  = '0023127efe52';  % valid DST mac-address of STA
% mac_address2  = '0018f8d2a29f';  % valid SRC mac-address of AP
% mac_address3  = '0018f8d2a29f';  % valid BSSID address of AP
% mac_seqctrl   = '0000';          % 0 sequence and 0 fragment number
% mac_crc       = '00000000';      % is calculated further down
% mac_body      = '0100';          % reason code 1 (who needs a reason :)


% Moluation/coding parameters
Regime         = regime;    % One of 8 possible regimes
switch Regime               % Regime-dependant parameters
  case 1                    % 3 Mbits/s *****MANDATORY*****
    Modulation = 'BPSK';    % BPSK modulation
    CodeRate   = 1/2;       % Coding rate 1/2 (unpunctured)
    RateBits   = [1 1 0 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 1;         % Coded bits per subcarrier (BPSK)
    Kmod       = 1;         % Normalization factor
  case 2                    % 4.5 Mbits/s
    Modulation = 'BPSK';    % BPSK modulation
    CodeRate   = 3/4;       % Coding rate 3/4 (punctured)
    RateBits   = [1 1 1 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 1;         % Coded bits per subcarrier (BPSK)
    Kmod       = 1;         % Normalization factor
  case 3                    % 6 Mbits/s *****MANDATORY*****
    Modulation = 'QPSK';    % QPSK modulation
    CodeRate   = 1/2;       % Coding rate 1/2 (unpunctured)
    RateBits   = [0 1 0 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 2;         % Coded bits per subcarrier (BPSK)
    Kmod       = 1/sqrt(2); % Normalization factor
  case 4                    % 9 Mbits/s
    Modulation = 'QPSK';    % QPSK modulation
    CodeRate   = 3/4;       % Coding rate 3/4 (punctured)
    RateBits   = [0 1 1 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 2;         % Coded bits per subcarrier (BPSK)
    Kmod       = 1/sqrt(2); % Normalization factor
  case 5                    % 12 Mbits/s *****MANDATORY*****
    Modulation = '16QAM';   % 16-QAM modulation
    CodeRate   = 1/2;       % Coding rate 1/2 (unpunctured)
    RateBits   = [1 0 0 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 4;         % Coded bits per subcarrier (QPSK)
    Kmod       = 1/sqrt(10);% Normalization factor
  case 6                    % 18 Mbits/s
    Modulation = '16QAM';   % 16-QAM modulation
    CodeRate   = 3/4;       % Coding rate 3/4 (punctured)
    RateBits   = [1 0 1 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 4;         % Coded bits per subcarrier (QPSK)
    Kmod       = 1/sqrt(10);% Normalization factor
  case 7                    % 24 Mbits/s
    Modulation = '64QAM';   % 64-QAM modulation
    CodeRate   = 2/3;       % Coding rate 2/3 (punctured)
    RateBits   = [0 0 0 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 6;         % Coded bits per subcarrier (64-QAM)
    Kmod       = 1/sqrt(42);% Normalization factor
  case 8                    % 27 Mbits/s
    Modulation = '64QAM';   % 64-QAM modulation
    CodeRate   = 3/4;       % Coding rate 3/4 (punctured)
    RateBits   = [0 0 1 1]; % RATE bits in the SIGNAL field
    Nbpsc      = 6;         % Coded bits per subcarrier (64-QAM)
    Kmod       = 1/sqrt(42);% Normalization factor
  otherwise
    error(['Unknown regime ' num2str(Regime)]);
end

nSubcarriers   = 48;                 % Number of data subcarriers
nPilots        = 4;                  % Number of pilot subcarriers
nPointsFFT     = 64;                 % Number of FFT points
Ncbps          = nSubcarriers*Nbpsc; % Coded bits per OFDM symbol
Ndbps          = Ncbps*CodeRate;     % Data bits per OFDM symbol
nGuardSamples  = nPointsFFT/4;       % Samples in the guard interval

% needed for generating the duration-bits 
dummy_packet = strcat(mac_framectrl,mac_duration,mac_address1,mac_address2,mac_address3,mac_seqctrl,mac_body,mac_crc);

% calculate duration bits
% the additional 2 at the end of this line is not in the standard!?
TxTime = 5*symbolduration + symbolduration * ceil((16 + 4 * length(dummy_packet) + 6)/Ndbps) + 2 
mac_duration  = dec2hex(TxTime,4);

% assemble MPDU (still without CRC32)
packet = strcat(mac_framectrl,mac_duration,mac_address1,mac_address2,mac_address3,mac_seqctrl,mac_body);

% change into transmit order
temp = '';

for i=1:length(packet)/2
   
    first_nibble = dec2bin(hex2dec(packet(2*i)),4);
    second_nibble = dec2bin(hex2dec(packet(2*i-1)),4);
    
    for t = 1:4
        first_nibble_rev(t) = first_nibble(5-t);
        second_nibble_rev(t) = second_nibble(5-t);
    end
    
    temp = strcat(temp,first_nibble_rev,second_nibble_rev);
    
end

MPDUData = 0;

% make logical
for i=1:length(temp)
    
    MPDUData(i)=logical(str2num(temp(i)));

end

%generate CRC
m = uint32(hex2dec('ffffffff'));
crc_m = uint32(hex2dec('edb88320'));

if exist('crc32_table') == 0
    disp('Creating crc32_table...');
    crc32_table = zeros(1,256,'uint32');
    for byte = 0:255
        crc = byte;
        for j = 1:8
            if bitand(crc,1) == 1
                mask = m;
            else
                mask = 0;
            end
            crc = bitxor(bitshift(crc,-1),bitand(crc_m, mask));
        end
        crc32_table(byte+1) = crc;
        dec2hex(crc32_table(byte+1));
    end
end

len = length(packet);
i = 1;
ff = uint32(hex2dec('ff'));
crc = m;

while i < len
    byte = uint32(hex2dec(packet(i:i+1)));
    t_index = bitand(bitxor(crc, byte), ff) + 1;
    crc = bitxor(bitshift(crc,-8), crc32_table(t_index));
    i = i+2;
end

crc = bitxor(crc,m);
crc_bin = dec2bin(crc,32);
crc = fliplr(crc_bin);
crc=logical((sscanf(crc,'%1d')).');

% optional: force the WRONG CRC32 for the reference frame in order to
% compare with the reference in ANNEX G of 802.11-2007
%crc = logical([0 1 0 1 1 0 1 1 1 1 1 0 1 0 1 0 1 0 0 1 1 0 0 1 1 0 1 1 0 1 1 1]);

% append the CRC32 at the end of the MPDU
TxData = [MPDUData crc];

%---------------------- generate short training sequence ------------------
shortseq = (1.472) * complex(1,1)* ...
           [0, 0, 1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0, ...
            0, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0];
X=zeros(nPointsFFT,1);                 % Empty input to the IFFT
% Map subcarriers
X(39:64) = shortseq(1:26);             % Map subcarriers -26 to -1
X(1)     = shortseq(27);               % Map DC subcarrier (not really necessary)
X(2:27)  = shortseq(28:53);            % Map subcarriers 1 to 26
% IFFT and scale
x=sqrt(nPointsFFT)*ifft(X,nPointsFFT); % IFFT
% Repeat for 2.5 times
short_preamble=[x;x;x(1:32)];

%---------------------- generate long training sequence -------------------
longseq = [1, 1,-1,-1, 1, 1,-1, 1,-1, 1, 1, 1, 1, 1, 1,-1,-1, 1, 1,-1, 1,-1, 1, 1, 1, 1, ...
           0, 1,-1,-1, 1, 1,-1, 1,-1, 1,-1,-1,-1,-1,-1, 1, 1,-1,-1, 1,-1, 1,-1, 1, 1, 1, 1];
X=zeros(nPointsFFT,1);                 % Empty input to the IFFT
% Map subcarriers
X(39:64) = longseq(1:26);              % Map subcarriers -26 to -1
X(1)     = longseq(27);                % Map DC subcarrier (not really necessary)
X(2:27)  = longseq(28:53);             % Map subcarriers 1 to 26
% IFFT and scale
x=sqrt(nPointsFFT)*ifft(X,nPointsFFT); % IFFT
% Cyclic prefix using block-diagonal matrix T (note: double length!)
I=eye(2*nPointsFFT);                           % Identitity matrix used to construct T
T=[I(2*nPointsFFT-2*nGuardSamples+1:end,:);I]; % Matrix for cyclic prefix insertion
% Prepend the prefix once to two copies of the sequence 
long_preamble=T*[x;x];

%----------------------- generate pilot sequences -------------------------
Pseq          = [...       % Pilot sequence of 127 BPSK symbols
  1,1,1,1, -1,-1,-1,1, -1,-1,-1,-1, 1,1,-1,1, -1,-1,1,1, -1,1,1,-1,...
  1,1,1,1, 1,1,-1,1, 1,1,-1,1, 1,-1,-1,1, 1,1,-1,1, -1,-1,-1,1,...
  -1,1,-1,-1, 1,-1,-1,1, 1,1,1,1, -1,-1,1,1, -1,-1,1,-1, 1,-1,1,1,...
  -1,-1,-1,1, 1,-1,-1,-1, -1,1,-1,-1, 1,-1,1,1, 1,1,-1,1, -1,1,-1,1,...
  -1,-1,-1,-1, -1,1,-1,1, 1,-1,1,-1, 1,1,1,-1, -1,1,-1,-1, -1,1,1,1,...
  -1,-1,-1,-1, -1,-1,-1
  ];

DataSize = ceil(size(TxData,2)/8);      % Payload size in bytes
nDataSym = ceil((DataSize*8+22)/Ndbps); % Number of data OFDM DATA symbols
plen = nDataSym+1;                      % Required length of pilot (=data+signal)
copies = ceil(plen/length(Pseq));       % Number of copies needed
temp_pilot = repmat(Pseq,1,copies);     % Generate enough copies
pilot1 = temp_pilot(1:plen);            % first pilot vector
pilot2 = temp_pilot(1:plen);            % second pilot vector
pilot3 = temp_pilot(1:plen);            % third pilot vector
pilot4 = temp_pilot(1:plen)*-1;         % fourth pilot vector (polarity inverted!)


%--- Construct signal-bitstream ------------------------------------
signal=logical(zeros(1,24));         % Empty SIGNAL field
signal(1:4)=RateBits;                % Insert the RATE bits
bytes=length(TxData)/8;              % Length of PSDU in bytes
lenstr=dec2bin(bytes,12);            % Convert to a binary (string!)
len=(sscanf(lenstr,'%1d')).';        % Length vector
signal(6:17)=logical(fliplr(len));   % Write the length bits, LSB first!
signal(18)=mod(sum(signal(1:17)),2); % even parity bit
%--- Construct data-bitstream --------------------------------------
dlen=ceil((length(TxData)+22)/Ndbps)*Ndbps; % Length of the DATA field, allocate 16bits service + 6 bits tail
data=logical(zeros(1,dlen));         % Empty DATA field
data(17:16+length(TxData))=TxData;   % Copy in the actual data 

%--- Scramble data-bitstream ---------------------------------------
scrambling_seq = [0 1 1 0 1 1 0 0 0 0 0 1 1 0 0 1 1 0 1 0 1 0 0 1 1 1 0 0 1 1 1 1,... 
                  0 1 1 0 1 0 0 0 0 1 0 1 0 1 0 1 1 1 1 1 0 1 0 0 1 0 1 0 0 0 1 1,... 
                  0 1 1 1 0 0 0 1 1 1 1 1 1 1 0 0 0 0 1 1 1 0 1 1 1 1 0 0 1 0 1 1,...
                  0 0 1 0 0 1 0 0 0 0 0 0 1 0 0 0 1 0 0 1 1 0 0 0 1 0 1 1 1 0 1'];
% Make copies of the short scramble-sequence and concatenate them
final_scrambling_seq = repmat(scrambling_seq,1,ceil(length(data)/127));
% Scramble the bullshit
data_scrambled = xor(data,final_scrambling_seq(1:length(data)));

% optional: turn off scrambling ------------------------------------
%data_scrambled = data;

% Insert the tail bits to make the conv-decoder at the receiver 
% return to 0 state at last bit of the crc 
data_scrambled(length(TxData)+17:length(TxData)+22) = logical([0 0 0 0 0 0]);

%--- Convolutional encoder -----------------------------------------
%--- Coding parameters ---------------------------------------------
K             = 7;         % Constraint length of the code
CodeGen       = [133 171]; % Code generator for the convolutional encoder
Trellis       = poly2trellis(K,CodeGen); % Encoder/decoder trellis
% Encode signal and data separately
signal_coded=convenc(signal,Trellis); % Encoding the SIGNAL field
data_coded=convenc(data_scrambled,Trellis);        % Encoding the DATA field
switch CodeRate                    % Need to puncture if the rate is not 1/2
  case 2/3
    data_coded(4:4:end)=[];                % Remove every 4th bit
  case 3/4
    data_coded(4:6:end)=[];
    data_coded(4:5:end)=[];                % Remove every 4th+5th bit
end

%--- Interleaver ---------------------------------------------------
%--- Signal field --------------------------------------------------
sin=reshape(signal_coded,48,[]); % Reshape into blocks of 48 bits
% First permutation
for k=0:47                       % Go through all the bits
  i=48/16*mod(k,16)+floor(k/16); % Calculate index
  stemp(i+1,:)=sin(k+1,:);       % Interleave
end                              % For loop (bits)
% Second permutation (not necessary as j=i for BPSK)
signal_interleaved=reshape(stemp,1,[]); % Just make into a long vector again
%--- Data field ----------------------------------------------------
din=reshape(data_coded,Ncbps,[]); % Reshape into blocks of Ncbps bits
% First permutation
for k=0:Ncbps-1                % Go through all the bits
  i=Ncbps/16*mod(k,16)+floor(k/16); % Calculate new index
  dtemp(i+1,:)=din(k+1,:);       % Interleave
end                              % For loop (bits)
% Second permutation
s=max(Nbpsc/2,1);              % Parameter used in index calculation below
for i=0:Ncbps-1                % Go through all the bits
  j=s*floor(i/s)+mod(i+Ncbps-floor(16*i/Ncbps),s); % Calculate new index
  dout(j+1,:)=dtemp(i+1,:);      % Interleave
end                              % For loop (bits)
data_interleaved=reshape(dout,1,[]); % Make into a long vector again


%--- Symbol mapper -------------------------------------------------
%--- Signal field (always BPSK) ------------------------------------
signal_mapped=qammod(double(signal_interleaved),2); % Just direct BPSK mapping
%----Data field ----------------------------------------------------
s=reshape(data_interleaved,Nbpsc,[]);  % Divide into symbols
d=zeros(1,size(s,2));        % Holder for decimal data values
for row=1:Nbpsc            % Iterate over rows of s
  d=d+s(row,:)...            % d is a weighted sum of rows of s
    *2^(Nbpsc-row);        % Weights are decreasing powers of 2
end                          % for loop (rows of s)
data_mapped=conj(qammod(d,2^Nbpsc,0,'gray'))*Kmod; % Map on the IQ plane

%--- Assemble the frame --------------------------------------------
frame=zeros(nSubcarriers+nPilots+1,(length(data_mapped)/nSubcarriers)+1);% One extra column for SIGNAL OFDM-symbol

%--- Insert the data subcarriers -----------------------------------
d=[signal_mapped.' reshape(data_mapped,nSubcarriers,[])]; % Concat SIGNAL and DATA
frame(1:5,1:end)  =d(1:5,:);   % Subcarriers -26 to -22
frame(7:19,1:end) =d(6:18,:);  % Subcarriers -20 to -8
frame(21:26,1:end)=d(19:24,:); % Subcarriers -6 to -1
frame(28:33,1:end)=d(25:30,:); % Subcarriers 1 to 6
frame(35:47,1:end)=d(31:43,:); % Subcarriers 8 to 20
frame(49:53,1:end)=d(44:48,:); % Subcarriers 22 to 26

%--- Insert the pilot subcarriers ----------------------------------
frame(6,1:end) =pilot1; % Subcarrier -21
frame(20,1:end)=pilot2; % Subcarrier -7
frame(34,1:end)=pilot3; % Subcarrier 7
frame(48,1:end)=pilot4; % Subcarrier 21

%--- OFDM modulation (IFFT) ----------------------------------------
X=zeros(nPointsFFT,size(frame,2)); % Empty input to the IFFT
I=eye(nPointsFFT);                 % Identitity matrix used to construct T
T=[I(nPointsFFT-nGuardSamples+1:end,:);I]; % Matrix for cyclic prefix insertion
% Map subcarriers accordingly
X(39:64,:)=frame(1:26,:);        % Map subcarriers -26 to -1
X(1,:)    =frame(27,:);            % Map DC subcarrier (not really necessary)
X(2:27,:) =frame(28:53,:);           % Map subcarriers 1 to 26
% Inverse fft
x=sqrt(nPointsFFT)*ifft(X,nPointsFFT); % IFFT
% Cyclic prefix using block-diagonal matrix T
Snd=T*x;

%--- Serialize -----------------------------------------------------
final = [reshape(Snd,1,size(Snd,1)*size(Snd,2))];
%--- Insert the long preamble sequence -----------------------------
final = [reshape(long_preamble,1,size(long_preamble,1)) final];
%--- Insert the short preamble sequences ---------------------------
final = [reshape(short_preamble,1,size(short_preamble,1)) final];
% scale down -------------------------------------------------------
final = 0.3*final;
% add some zeros at the end ----------------------------------------
final = [final zeros(1,1040)]; 

% plot the result --------------------------------------------------
figure;
subplot(2,1,1);plot(abs(final),'b');
xlabel('sample');
ylabel('magnitude');
subplot(2,1,2);plot(unwrap(angle(final)),'b');
xlabel('sample');
ylabel('phase');

% write to file - interleaved, real parts first --------------------
file_name='matlabframe.dat';
fid = fopen(file_name,'w');
new_save = reshape([real(final);imag(final)],1,2*length(final));
F = fwrite(fid,new_save,'float');
fclose all;

% % optional: Compare with reference given by 802.11a-1999.pdf ----
% 
% % you will see differences at the OFDM symbol boundaries, this is due to
% % the time-windowing that is done in the reference frame but is not part of
% % the standard: "The time-windowing feature illustrated here is not part 
% % of the normative specifications." See page 1082.
% 
% % read the reference from the standard document table G.24
% original = textread('802.11-2007-Annex-G.8.txt','%s','delimiter','\n');
% 
% % process the ASCII file input
% reference_real = zeros(881,1);
% reference_imag = zeros(881,1);
% 
% for i = 1:881
%     
%     ref_real(i) = str2num(cell2mat(original(i*3-1)));
%     ref_imag(i) = str2num(cell2mat(original(i*3)));
% 
% end
% 
% ref_final = 2.4*complex(ref_real,ref_imag)
% 
% figure;
% plot(abs(final(1:880)-ref_final(1:880)),'r');
% title('Difference to reference frame');
