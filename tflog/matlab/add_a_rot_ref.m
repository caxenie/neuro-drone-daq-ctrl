function [ld_out] = add_a_rot_ref(ld)

if(isfield(ld,'imu') == 0)
    ld_out = ld;
    return;
end


%**************************************************************************
%%      rotational-gravitational acceleration reference
%           attitude / tracker based reference
%**************************************************************************

% EKF
if(isfield(ld,'att') == 0)
    ld_out = ld;
    return;
end
t       = ld.att.hrt.t;
roll    = -ld.att.roll;
pitch   = -ld.att.pitch;
yaw     = ld.att.yaw - ld.yaw_off;

% Tracker
% if(isfield(ld,'rb') == 0)
%     ld_out = ld;
%     return;
% end
% t       = ld.rb.hrt.t;
% roll    = ld.rb.roll;
% pitch   = -ld.rb.pitch;
% yaw     = -ld.rb.yaw;


% interpolate data to imu hrt timeline
roll    = interp1(t,  roll,     ld.imu.hrt.t);
pitch   = interp1(t,  pitch,    ld.imu.hrt.t);
yaw     = interp1(t,  yaw,      ld.imu.hrt.t);

n       = ld.imu.n;
g       = [zeros(1,n);zeros(1,n);-ones(1,n)*ld.g];


% calculate reference for gravitational accereleration reference
ld.a_rot_ref = vec_inv_rot_euler_rpy_rad(roll, pitch, yaw, g);

ld_out = ld;

end


function [deg] = toDeg(rad)
deg = rad/pi*180;
end


function [rad] = toRad(deg)
rad = deg/180*pi;
end


function [v_hat] = vec_inv_rot_euler_rp_rad(roll, pitch, v)
n = length(roll);
v_hat=zeros(3,n);

roll = toDeg(roll);
pitch = toDeg(pitch);

for k=1:n
  
    if(roll(k) > -180 || roll(k) < 180 )
        v_hat(:,k) = rotx(roll(k)) * v(:,k);
    end
    
    if(pitch(k) > -180 || pitch(k) < 180 )
        v_hat(:,k) = roty(pitch(k)) * v_hat(:,k);
    end
end

end

% use this function to transform from drone to world coord.
%   if roll,pitch,yaw are correct, that the g Vector is parallel
%   to the z axis of the drone coord. sys
function [v_hat] = vec_inv_rot_euler_rpy_rad(roll, pitch, yaw, v)
n = length(roll);
v_hat=zeros(3,n);

roll = toDeg(roll);
pitch = toDeg(pitch);
yaw = toDeg(yaw);

for k=1:n
  
    if(roll(k) > -180 || roll(k) < 180 )
        v_hat(:,k) = rotx(roll(k)) * v(:,k);
    end
    
    if(pitch(k) > -180 || pitch(k) < 180 )
        v_hat(:,k) = roty(pitch(k)) * v_hat(:,k);
    end
    
    if(yaw(k) > -180 || yaw(k) < 180 )
        v_hat(:,k) = rotz(yaw(k)) * v_hat(:,k);
    end
end

end