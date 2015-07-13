%main
clear all
close all


% dir_name = 'log-20140127182842-tf1';
dir_name = 'log-20140127183058-tf2';
% dir_name = 'log-20140127183230-tf3';

% dir_name = 'log-20140205111625-roll-test';    % without acc


% dir_name = 'log-20140205125555-roll-test-2';    % with z axis acc
% dir_name = 'log-20140205131951-ax-test';        % ax test (no rotations)
% dir_name = 'log-20140205132536-ay-test';        % ay test (no rotations)
% dir_name = 'log-20140205133431-az-test';        % az test (no rotations)


%dir_name = 'log-20140206110541-tfp-yaw-test';

ldata = read_log_data('../logs',dir_name);

ld = add_precalcs(ldata);

plot_time_analysis(ld);


% figure
% plot_servo_output_raw(ld.sor);


% figure
% plot_manual_control(ld.mc);


% figure
% plot_sys_status(ld.ss);


% figure
% plot_highres_imu(ld.imu, ld.tsmin);


% figure
% plot_attitude(ld.att, ld.tsmin);


% figure
% plot_optical_flow(ld.of, ld.tsmin);


% figure
% plot_rigidBody(ld.rb, ld.tsmin);



% figure 
% plot_rigidBody_lin_trans(ld);


% figure
% plot_of_weighted(ld);


% figure
% plot_of_v_lin(ld);



% figure
% plot_acc_filtered(ld);


% figure
% plot_acc_angle_vel(ld);

% 
% figure
% plot_acc_LPF_and_FT(ld);


% figure
% plot_acc_a_lin(ld);


% figure
% plot_acc_a_rot(ld);


% figure
% plot_mag_b(ld);

% 
% figure
% plot_rpy(ld);


% figure
% plot_rpy_gyro(ld);


% figure
% plot_rpy_acc(ld);


% figure
% plot_rpy_mag(ld);


% figure
% plot_KF_roll(ld);



% figure
% plot_time_analysis(ld);



% figure
% plot_roll_spectrogram(ld);






