file_name='../examples/final.dat';
fid = fopen(file_name,'r');
F = fread(fid,'float');

gnu_final = complex(F(1:2:end)',F(2:2:end)');

subplot(2,1,1);hold;plot(abs(final-gnu_final),'r')

subplot(2,1,2);hold;plot(unwrap(angle(final)-angle(gnu_final)),'r')
fclose all;
