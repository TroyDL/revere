
#include "test_r_db_r_sqlite_pager.h"
#include "r_db/r_sqlite_pager.h"
#include "r_db/r_sqlite_conn.h"

using namespace r_db;

REGISTER_TEST_FIXTURE(test_r_db_r_sqlite_pager);

void test_r_db_r_sqlite_pager::setup()
{
    remove("test.db");
}

void test_r_db_r_sqlite_pager::teardown()
{
    remove("test.db");
}

void test_r_db_r_sqlite_pager::test_next()
{
    r_sqlite_conn conn("test.db");

    r_sqlite_transaction(conn, [](const r_sqlite_conn& conn){
        conn.exec("CREATE TABLE worker_bees(sesa_id TEXT, name TEST, id INTEGER PRIMARY KEY AUTOINCREMENT);");
        conn.exec("CREATE INDEX worker_bees_sesa_id_idx ON worker_bees(sesa_id);");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123456', 'Alan Turing');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123458', 'John Von Neumann');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123460', 'Alonzo Church');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123462', 'Donald Knuth');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123464', 'Dennis Ritchie');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123466', 'Claude Shannon');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123468', 'Bjarne Stroustrup');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123470', 'Linus Torvalds');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123472', 'Larry Wall');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123474', 'Yann LeCun');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123476', 'Yoshua Bengio');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123478', 'Guido van Rossum');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123480', 'Edsger W. Dijkstra');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123482', 'Ken Thompson');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123484', 'Grace Hopper');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123486', 'Ada Lovelace');");
    });

    r_sqlite_pager p("*", "worker_bees", "sesa_id", 4);

    p.find(conn);
    auto row = p.current();
    RTF_ASSERT(row["name"] == "Alan Turing");
    RTF_ASSERT(p.valid());
    p.next(conn);
    
    row = p.current();
    RTF_ASSERT(row["name"] == "John Von Neumann");
    RTF_ASSERT(p.valid());
    p.next(conn);
    
    row = p.current();
    RTF_ASSERT(row["name"] == "Alonzo Church");
    RTF_ASSERT(p.valid());
    p.next(conn);
    
    row = p.current();
    RTF_ASSERT(row["name"] == "Donald Knuth");
    RTF_ASSERT(p.valid());
    p.next(conn);

    // should fetch next page at this point...

    row = p.current();
    RTF_ASSERT(row["name"] == "Dennis Ritchie");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Claude Shannon");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Bjarne Stroustrup");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Linus Torvalds");
    RTF_ASSERT(p.valid());
    p.next(conn);
    
    // should fetch next page at this point...

    row = p.current();
    RTF_ASSERT(row["name"] == "Larry Wall");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Yann LeCun");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Yoshua Bengio");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Guido van Rossum");
    RTF_ASSERT(p.valid());
    p.next(conn);

    // should fetch next page at this point...

    row = p.current();
    RTF_ASSERT(row["name"] == "Edsger W. Dijkstra");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Ken Thompson");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Grace Hopper");
    RTF_ASSERT(p.valid());
    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Ada Lovelace");
    RTF_ASSERT(p.valid());

    p.next(conn); // NOTE: this should knock us out of valid() == true because our query will return empty results.

    RTF_ASSERT(p.valid() == false);
}

void test_r_db_r_sqlite_pager::test_find()
{
    r_sqlite_conn conn("test.db");

    r_sqlite_transaction(conn, [](const r_sqlite_conn& conn){
        conn.exec("CREATE TABLE worker_bees(sesa_id TEXT, name TEST, id INTEGER PRIMARY KEY AUTOINCREMENT);");
        conn.exec("CREATE INDEX worker_bees_sesa_id_idx ON worker_bees(sesa_id);");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123456', 'Alan Turing');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123458', 'John Von Neumann');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123460', 'Alonzo Church');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123462', 'Donald Knuth');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123464', 'Dennis Ritchie');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123466', 'Claude Shannon');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123468', 'Bjarne Stroustrup');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123470', 'Linus Torvalds');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123472', 'Larry Wall');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123474', 'Yann LeCun');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123476', 'Yoshua Bengio');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123478', 'Guido van Rossum');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123480', 'Edsger W. Dijkstra');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123482', 'Ken Thompson');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123484', 'Grace Hopper');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123486', 'Ada Lovelace');");
    });

    r_sqlite_pager p("*", "worker_bees", "sesa_id", 4);

    p.find(conn, "123467"); // non exact match

    auto row = p.current();
    RTF_ASSERT(row["name"] == "Bjarne Stroustrup");
    RTF_ASSERT(p.valid());
    p.next(conn);

    p.find(conn, "123484"); // exact match + less than 4 results
    
    row = p.current();
    RTF_ASSERT(row["name"] == "Grace Hopper");
    RTF_ASSERT(p.valid());

    p.next(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Ada Lovelace");
    RTF_ASSERT(p.valid());

    p.next(conn); // should invalidate us...

    RTF_ASSERT(!p.valid());
}

void test_r_db_r_sqlite_pager::test_prev()
{
    r_sqlite_conn conn("test.db");

    r_sqlite_transaction(conn, [](const r_sqlite_conn& conn){
        conn.exec("CREATE TABLE worker_bees(sesa_id TEXT, name TEST, id INTEGER PRIMARY KEY AUTOINCREMENT);");
        conn.exec("CREATE INDEX worker_bees_sesa_id_idx ON worker_bees(sesa_id);");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123456', 'Alan Turing');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123458', 'John Von Neumann');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123460', 'Alonzo Church');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123462', 'Donald Knuth');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123464', 'Dennis Ritchie');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123466', 'Claude Shannon');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123468', 'Bjarne Stroustrup');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123470', 'Linus Torvalds');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123472', 'Larry Wall');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123474', 'Yann LeCun');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123476', 'Yoshua Bengio');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123478', 'Guido van Rossum');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123480', 'Edsger W. Dijkstra');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123482', 'Ken Thompson');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123484', 'Grace Hopper');");
        conn.exec("INSERT INTO worker_bees(sesa_id, name) VALUES('123486', 'Ada Lovelace');");
    });

    r_sqlite_pager p("*", "worker_bees", "sesa_id", 4);

    p.find(conn, "123460"); // exact match

    auto row = p.current();
    RTF_ASSERT(row["name"] == "Alonzo Church");
    RTF_ASSERT(p.valid());
    p.prev(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "John Von Neumann");
    RTF_ASSERT(p.valid());
    p.prev(conn);

    row = p.current();
    RTF_ASSERT(row["name"] == "Alan Turing");
    RTF_ASSERT(p.valid());
    p.prev(conn); // should invalidate us...

    RTF_ASSERT(!p.valid());
}
