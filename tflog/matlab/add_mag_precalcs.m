function [ld_out] = add_mag_precalcs(ld)

if(isfield(ld,'imu') == 0)
    ld_out = ld;
    return;
end

ld.mag.bx = ld.imu.xmag;
ld.mag.by = ld.imu.ymag;
ld.mag.bz = ld.imu.zmag;

ld.mag.b = [ld.imu.xmag';ld.imu.ymag';ld.imu.zmag'];

% EKF
if(isfield(ld,'att') == 0)
    ld_out = ld;
    return;
end

t       = ld.att.hrt.t;
roll    = -ld.att.roll;
pitch   = -ld.att.pitch;


% Tracker
% if(isfield(ld,'rb') == 0)
%     ld_out = ld;
%     return;
% end
% t       = ld.rb.hrt.t;
% roll    = ld.rb.roll;
% pitch   = -ld.rb.pitch;


% interpolate data to imu timeline
roll    = interp1(t,  roll,     ld.imu.hrt.t);
pitch   = interp1(t,  pitch,    ld.imu.hrt.t);


% undo roll and pitch transformations
b_hat = vec_inv_rot_euler_rp(toDeg(-roll),toDeg(-pitch), ld.mag.b);

bx = b_hat(1,:);
by = b_hat(2,:);

ld.mag.yaw      = atan2(by, bx);
ld.mag.yaw_off  = mean(ld.mag.yaw(1:100));


ld.mag.yaw_f = filter_bw_LP(ld.mag.yaw,1,10,ld.imu.hrt.freq_mean);

ld_out = ld;

end





function [y] = filter_bw_LP(x, N, Fc, Fs)

[z,p,k] = butter(N,Fc/(Fs/2),'low');
[sos,g] = zp2sos(z,p,k);	    % Convert to SOS form
Hf = dfilt.df2tsos(sos,g);      % Create a dfilt object

y = filter(Hf,x);

end

function [deg] = toDeg(rad)
deg = rad/pi*180;
end


function [rad] = toRad(deg)
rad = deg/180*pi;
end


function [b_hat] = vec_inv_rot_euler_rp(roll, pitch, b)
n = length(roll);
b_hat=zeros(3,n);

for k=1:n
    
    % b_hat(:,k) = roty(pitch(k))*rotx(roll(k)) * b(:,k);
    
    if(roll(k) > -180 || roll(k) < 180 )
        b_hat(:,k) = rotx(roll(k)) * b(:,k);
    end
    
    if(pitch(k) > -180 || pitch(k) < 180 )
        b_hat(:,k) = roty(pitch(k)) * b_hat(:,k);
    end
end

end