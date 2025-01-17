CREATE TABLE employee (
	employee_id INT,
	first_name VARCHAR(100),
	last_name VARCHAR(100),
	gender VARCHAR(10),
	salary INT,
	birth_date DATE,
	department_id INT,
	PRIMARY KEY (employee_id),
	FOREIGN KEY (department_id) REFERENCES department (department_id)
);

CREATE TABLE department (
	department_id INT,
	name VARCHAR(50),
	location VARCHAR(50),
	PRIMARY KEY (department_id)
);











