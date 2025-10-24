create table employee (
	employee_id int,
	first_name varchar(100),
	last_name varchar(100),
	gender varchar(10),
	salary int,
	birth_date date,
	department_id int,
	primary key (employee_id),
	foreign key (department_id) references department (department_id)
)
create table department (
	department_id int,
	name varchar(50),
	location varchar(50),
	primary key (department_id)
)
create table user (
	user_id int auto_increment,
	name varchar(100),
	email varchar(100) unique,
	password varchar(100),
	primary key (user_id)
)
create table customer (
	customer_id int auto_increment,
	name varchar(100),
	address varchar(100),
	phone varchar(10) unique,
	primary key (customer_id)
)
create table product (
	product_id int auto_increment,
	name varchar(100),
	qty int default 0,
	price float,
	kind varchar(100),
	primary key (product_id)
)
create table order (
	order_id int auto_increment,
	created_on datetime default now(),
	customer_id int,
	product_id int,
	primary key (order_id),
	foreign key (customer_id) references customer (customer_id),
	foreign key (product_id) references product (product_id)
)














