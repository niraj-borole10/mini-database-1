# Mini SQL-like Database Engine

This project is a simple, custom database engine written in C++ (C++11 compatible). It supports basic SQL-like queries from the command line.

## Features

- Create tables with columns of type `INT`, `STRING`, or `FLOAT`  <!-- Added FLOAT here -->
- Insert new records
- Select all records from a table
- Delete records based on a condition
- Update records based on a condition
- Simple SQL-like query parsing

## How to Use

1. **Build the project** (see instructions in your `CMakeLists.txt`).
2. **Run the executable**. You will see a prompt:
   ```
   Welcome to the Mini Database Engine!
   Type your queries (type EXIT to quit):
   >
   ```

3. **Type your queries** as described below.

---

## Supported Query Structure

### 1. Create Table

```
CREATE TABLE table_name (column1 INT, column2 STRING, column3 FLOAT, ...)
```
**Example:**
```
CREATE TABLE users (id INT, name STRING, balance FLOAT)
```

---

### 2. Insert Data

```
INSERT INTO table_name VALUES (value1, value2, value3, ...)
```
**Example:**
```
INSERT INTO users VALUES (1, 'Alice', 100.5)
```

---

### 3. Select Data

```
SELECT * FROM table_name
```
**Example:**
```
SELECT * FROM users
```

---

### 4. Delete Data

```
DELETE FROM table_name WHERE column = value
```
**Example:**
```
DELETE FROM users WHERE balance = 100.5
```

---

### 5. Update Data

```
UPDATE table_name SET column = value WHERE column = value
```
**Example:**
```
UPDATE users SET balance = 200.0 WHERE id = 1
```

---

### 6. Exit

```
EXIT
```

---

## Notes

- String values must be enclosed in single quotes (`'Alice'`).
- Supported column types: `INT`, `STRING`, and `FLOAT`. 
- All queries must end with a newline (press Enter).

---

## Example Session

```
CREATE TABLE users (id INT, name STRING, balance FLOAT)
INSERT INTO users VALUES (1, 'Alice', 100.5)
INSERT INTO users VALUES (2, 'Bob', 200.0)
SELECT * FROM users
UPDATE users SET balance = 300.0 WHERE id = 2
DELETE FROM users WHERE name = 'Alice'
SELECT * FROM users
EXIT
```
**Command To Run**

g++ -std=c++11 src/*.cpp -Iinclude -o DBEngine

---

# Mini Database API

This project provides a RESTful API for a simple in-memory database using Crow (C++ micro web framework). Below are the supported endpoints, their usage, and the expected request/response formats.

---

## Endpoints and Query Formats

### 1. Health Check & Logs

**GET /**  
Returns the contents of `performance_metrics.log`.

---

### 2. List All Tables

**GET /tables**  
**Response:**  
```json
{
  "tables": ["table1", "table2", ...],
  "duration_us": 123
}
```

---

### 3. Create a New Table

**POST /tables**  
**Request Body:**  
```json
{
  "name": "table_name",
  "columns": [
    { "name": "col1", "type": "INT" },
    { "name": "col2", "type": "STRING" },
    { "name": "col3", "type": "FLOAT" }
  ]
}
```
**Response:**  
- `201 Created` on success
- `400 Bad Request` if missing fields or unknown type

---

### 4. Insert Data into a Table

**POST /tables/{table}/rows**  
**Request Body:**  
A JSON array of values, matching the order and type of the table's columns.
```json
[1, "hello", 3.14]
```
**Response:**  
- `201 Created` on success
- `400 Bad Request` if wrong number/type of fields

---

### 5. Query Data from a Table

**GET /tables/{table}/rows**  
**Response:**  
```json
{
  "rows": [
    { "col1": 1, "col2": "hello", "col3": 3.14 },
    ...
  ],
  "duration_us": 123
}
```
- `404 Not Found` if table does not exist

---

### 6. Delete a Table

**DELETE /tables/{table}**  
(Not implemented, returns 501)

---

### 7. Delete a Row by Index

**DELETE /tables/{table}/rows/{row_id}**  
Deletes the row at the given index (0-based).
- `200 OK` on success
- `404 Not Found` if table or row does not exist

---

### 8. Alter Table (Add/Drop Column)

**POST /tables/{table}/columns**  
**Request Body for Add:**  
```json
{
  "action": "add",
  "column": "new_col",
  "type": "INT"
}
```
**Request Body for Drop:**  
```json
{
  "action": "drop",
  "column": "col_to_remove"
}
```
- `200 OK` on success
- `400 Bad Request` for missing/invalid fields

---

### 9. Delete Rows Where Condition

**POST /tables/{table}/delete**  
**Request Body:**  
```json
{
  "column": "col_name",
  "value": "value_to_match"
}
```
Deletes all rows where `col_name == value_to_match`.

---

### 10. Update Rows Where Condition

**POST /tables/{table}/update**  
**Request Body:**  
```json
{
  "set_column": "col_to_update",
  "set_value": "new_value",
  "where_column": "col_to_match",
  "where_value": "value_to_match"
}
```
Updates all rows where `where_column == where_value`, setting `set_column` to `set_value`.

---

## Data Types

- `"INT"`: Integer
- `"FLOAT"`: Floating point number
- `"STRING"`: String

---

## Notes

- All durations are returned in microseconds (`duration_us`).
- All errors are returned as plain text or as JSON with an `"error"` field.
- All logs are appended to `performance_metrics.log` in the project directory.

**Command to run server**
g++ -std=c++17 -Iinclude -o web_server web_server.cpp src/*.cpp -lpthread -lws2_32 -lmswsock

---