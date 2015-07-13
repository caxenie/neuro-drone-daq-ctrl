function [ld_out] = add_acc_precalcs(ld)

if(isfield(ld,'imu') == 0)
    ld_out = ld;
    return;
end


ld.acc.ax = ld.imu.xacc;
ld.acc.ay = ld.imu.yacc;
ld.acc.az = ld.imu.zacc;

ld.acc.ax_off = mean(ld.acc.ax(1:100));
ld.acc.ay_off = mean(ld.acc.ay(1:100));

ld.acc.ax = ld.acc.ax - ld.acc.ax_off;
ld.acc.ay = ld.acc.ay - ld.acc.ay_off;


% get net linear acceleration (graviational + translational linear acceleration)
ld.acc.a = [ld.acc.ax';ld.acc.ay';ld.acc.az'];

% low-pass filter net linear acceleration
ld.acc.a_f = filter_vec_bw_LP(ld.acc.a, 2, 3, ld.imu.hrt.freq_mean);

% calculate norm of net linear acceleration
ld.acc.a_norm = tline_norm(ld.acc.a);


% filter components of net linear acceleration in a different manner
Fc = 2;
ld.acc.ax_f = filter_bw_LP(ld.acc.ax, 2, Fc, ld.imu.hrt.freq_mean);
ld.acc.ay_f = filter_bw_LP(ld.acc.ay, 2, Fc, ld.imu.hrt.freq_mean);
ld.acc.az_f = filter_bw_LP(ld.acc.az, 2, 0.8, ld.imu.hrt.freq_mean);


ax = ld.acc.ax_f;
ay = ld.acc.ay_f;
az = ld.acc.az_f;

g = ld.g;

% decides which points are on a sphere with radius g and threshold 0.1g
ld.acc.valid = sphericalFilter(ax,ay,az, g, 0.1*g);


% calculate roll and pitch angles
ax = ld.acc.ax_f;
ay = ld.acc.ay_f;
az = ld.acc.az_f;

ld.acc.raw.roll     = -atan2(ay, -az);
ld.acc.raw.pitch    = atan2(ax, -az);


% n = ld.imu.n;
% 
% for k=1:n
%     
%     roll = ld.acc.raw.roll(k);
%     z = ay(k)*sin(roll) + -az(k)*cos(roll);
% 
%     ld.acc.raw.pitch(k) = atan2(ax(k), z);
% 
% end


ax = ld.acc.ax;
ay = ld.acc.ay;
az = ld.acc.az;

ld.acc.raw.uf.roll     = -atan2(ay, -az);
ld.acc.raw.uf.pitch    = atan2(ax, -az);



ld_out = ld;

end


function [y] = filter_bw_LP(x, N, Fc, Fs)

[z,p,k] = butter(N,Fc/(Fs/2),'low');
[sos,g] = zp2sos(z,p,k);	    % Convert to SOS form
Hf = dfilt.df2tsos(sos,g);      % Create a dfilt object

y = filter(Hf,x);

end

function [y] = filter_running_average(x, N)
y = filter(ones(1,N)/N,1,x);
end


function [vec_f] = filter_vec_bw_LP(vec, N, Fc, Fs)

[z,p,k] = butter(N,Fc/(Fs/2),'low');
[sos,g] = zp2sos(z,p,k);	    % Convert to SOS form
Hf = dfilt.df2tsos(sos,g);      % Create a dfilt object

vec_f(1,:) = filter(Hf,vec(1,:));
vec_f(2,:) = filter(Hf,vec(2,:));
vec_f(3,:) = filter(Hf,vec(3,:));

end


function [vec_f] = filter_vec_FIR_LP(vec, N, Fc, Fs)

d = fdesign.lowpass('N,Fc',N,Fc,Fs);
Hf = design(d,'FIR');

vec_f(1,:) = filter(Hf,vec(1,:));
vec_f(2,:) = filter(Hf,vec(2,:));
vec_f(3,:) = filter(Hf,vec(3,:));

end



function [vec_f] = filter_vec_running_average(vec, N)

vec_f(1,:) = filter(ones(1,N)/N,1,vec(1,:));
vec_f(2,:) = filter(ones(1,N)/N,1,vec(2,:));
vec_f(3,:) = filter(ones(1,N)/N,1,vec(3,:));

end


% quick and dirty way to ignore acc vectors > g
function [valid] = sphericalFilter(x,y,z,r,delta)

r_hat = x.*x + y.*y + z.*z;

d1 = r - delta;
d2 = r + delta;

sqdelta1 = d1*d1;
sqdelta2 = d2*d2;


valid = ones(size(x));

for k=1:length(x)
    
    if(r_hat(k) < sqdelta1 || r_hat(k) > sqdelta2)
        valid(k) = 0;  % ignore
    end
end

valid = logical(valid);

end





function [norm] = tline_norm(vec)

n = size(vec,2);
norm = zeros(n,1);

for l =1:n    
    norm(l) = sqrt(vec(:,l)'*vec(:,l));
end

end







