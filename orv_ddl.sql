drop table if exists fv_schedule_state;
drop table if exists fv_schedule;
drop table if exists fv_payment;
drop table if exists fv_rate;
drop table if exists fv_sozaem;
drop table if exists fv_agr;
drop table if exists orv_member;

PRAGMA foreign_keys = ON;

create table orv_member(
 id integer not null primary key autoincrement,
 fio text not null check( fio !='' ),
 pol_m boolean,
 pasp_ser text,
 pasp_num text,
 pasp_dvyd date,
 pasp_kem_vyd text,
 addr_reg text,
 contact text,
 rel_code text,
 fv_zayavka bigint,
 comment text
);

create table fv_agr(
 id integer not null primary key autoincrement,
 depositor_id integer not null,
 closed date,
 foreign key (depositor_id) references orv_member(id) on delete restrict on update cascade
);

create table fv_sozaem(
 agr_id integer not null,
 member_id integer not null,
 type_id integer   not null check( type_id in (1, 2) ),
 PRIMARY KEY(agr_id, member_id),
 foreign key (agr_id) references fv_agr(id) on delete cascade on update cascade
 foreign key (member_id) references orv_member(id) on delete restrict on update cascade
);

create table fv_rate(
 agr_id integer not null,
 start_date date not null,
 type_id integer not null check( type_id in (1, 2) ),
 value double not null,
 PRIMARY KEY(agr_id, start_date, type_id),
 foreign key (agr_id) references fv_agr(id) on delete cascade on update cascade
);

create table fv_payment(
 id integer not null primary key autoincrement,
 agr_id integer not null,
 state_date date not null,
 all_payment bigint not null default 0,
 overdue_remainder bigint not null default 0,
 overdue_interest bigint not null default 0,
 overdue_payment bigint not null default 0,
 comment text,
 foreign key (agr_id) references fv_agr(id) on delete cascade on update cascade
);

create table fv_schedule(
 payment_id integer not null primary key,
 reason_id integer not null check( reason_id in (1, 2) ),
 foreign key (payment_id) references fv_payment(id) on delete cascade on update cascade  
);

create table fv_schedule_state(
 schedule_id integer not null,
 state_date date not null,
 remainder bigint not null,
 interest bigint not null,
 payment bigint not null,
 comment text,
 payment_id integer,
 PRIMARY KEY(schedule_id, state_date),
 foreign key (schedule_id) references fv_schedule(payment_id) on delete cascade on update cascade  
 foreign key (payment_id) references fv_payment(id) on delete set null on update cascade  
);


