function [ld_out] = add_gyro_precalcs(ld)

if(isfield(ld,'imu') == 0)
    ld_out = ld;
    return;
end

n = ld.imu.n;

roll   = zeros(n,1);
pitch  = zeros(n,1);
yaw    = zeros(n,1);


% numerically integrate angular velocities
for k=2:n
    roll(k)     = roll(k-1)   + ld.imu.xgyro(k-1) * ld.imu.hrt.dt(k);
    pitch(k)    = pitch(k-1)  + ld.imu.ygyro(k-1) * ld.imu.hrt.dt(k);
    yaw(k)      = yaw(k-1)    + ld.imu.zgyro(k-1) * ld.imu.hrt.dt(k);
end


ld.gyro.raw.or = [roll';pitch';yaw'];

ld.gyro.raw.roll = roll;
ld.gyro.raw.pitch = pitch;
ld.gyro.raw.yaw = yaw;


ld_out = ld;

end