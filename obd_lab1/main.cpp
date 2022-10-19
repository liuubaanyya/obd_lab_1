#include <io.h>
#include <iostream>
#include <cstring>

using std::cin;
using std::cout;
using std::endl;

const long MAXROWS = 50;
const int C1 = 114750541;
const int C2 = 4751951;
const int MAXLEN = 20;

char im[15] = "mIndex.bin";
char is[15] = "sIndex.bin";
char tm[15] = "mTable.bin";
char ts[15] = "sTable.bin";
char nm[15] = "mInfo.bin";
char ns[15] = "sInfo.bin";

struct Customer {
    char id[MAXLEN];
    char name[MAXLEN];
    char birthday[MAXLEN];
};
struct Order {
    char id[MAXLEN];
    char status[MAXLEN];
    char date[MAXLEN];
    unsigned long custID;
};
struct forIndTable {
    bool del; // 1 - is deleted
    unsigned long key;
    long ind;
    long forList;
};
struct tableInfo {
    FILE* tablePtr;
    forIndTable* indT;
    long forDel;
    long capacity;
    long total;
    long count;
};

tableInfo customers, orders;

void dread(FILE* fp, void* buf, const int size, const long offset) {
    fseek(fp, size * offset, SEEK_SET);
    fread(buf, size, 1, fp);
}
void dwrite(FILE* fp, void* buf, const int size, const long offset) {
    fseek(fp, size * offset, SEEK_SET);
    fwrite(buf, size, 1, fp);
    fflush(fp);
}
unsigned long convert(const char* str) {
    return strtoul(str, NULL, 10);
}

FILE* dopenTable(const char* name, const long size) {
    FILE* fp;
    fopen_s(&fp, name, "rb+");
    if (!fp) {
        fopen_s(&fp, name, "wb+");
        _chsize_s(_fileno(fp), size * MAXROWS);
    }
    return fp;
}
void updInf(const char* name, tableInfo* t) {
    FILE* fp;
    fopen_s(&fp, name, "rb");
    if (!fp) {
        t->count = t->total = t->forDel = 0;
        t->capacity = MAXROWS;
    }
    else {
        dread(fp, &t->count, sizeof(long), 0);
        dread(fp, &t->forDel, sizeof(long), 1);
        t->total = t->count;

        t->capacity = ((long)(t->total / MAXROWS) + 1) * MAXROWS;

        fclose(fp);
    }
}
void dopenInd(const char* name, tableInfo* t) {
    t->indT = (forIndTable*)malloc(t->capacity * sizeof(forIndTable));

    FILE* fp;
    fopen_s(&fp, name, "rb");
    if (fp) {
        for (long i = 0; i < t->total; ++i)
            dread(fp, &t->indT[i], sizeof(forIndTable), i);

        fclose(fp);
    }
}
void dopen() {
    customers.tablePtr = dopenTable(tm, sizeof(Customer));
    updInf(nm, &customers);
    dopenInd(im, &customers);

    orders.tablePtr = dopenTable(ts, sizeof(Order));
    updInf(ns, &orders);
    dopenInd(is, &orders);
}

void pubPrint(const Customer* tmp) {
    cout.width(MAXLEN); cout << tmp->id << ' ';
    cout.width(MAXLEN); cout << tmp->name << ' ';
    cout.width(MAXLEN); cout << tmp->birthday << ' ';
}
void edPrint(const Order* tmp) {
    cout.width(MAXLEN); cout << tmp->id << ' ';
    cout.width(MAXLEN); cout << tmp->status << ' ';
    cout.width(8); cout << tmp->date << ' ';
    cout.width(MAXLEN); cout << tmp->custID << ' ';
}
void indPrint(const tableInfo* t) {
    cout << "INDEX TABLE" << endl << "deleted - key - index - first/next" << endl;
    for (int i = 0; i < t->total; ++i)
        cout << t->indT[i].del << " | " << t->indT[i].key << " | " << t->indT[i].ind << " | " << t->indT[i].forList << endl;
}
void mPrint() {
    cout << "CUSTOMERS" << endl;
    cout.width(MAXLEN); cout << "ID";
    cout.width(MAXLEN + 1); cout << "NAME";
    cout.width(MAXLEN + 1); cout << "BIRTHDAY" << endl;

    Customer tmp;
    for (int i = 0; i < customers.total; ++i) {
        if (!customers.indT[i].del) {
            dread(customers.tablePtr, &tmp, sizeof(Customer), customers.indT[i].ind);
            cout << endl << "--------------------------------------------------------------------------------------------------" << endl;
            pubPrint(&tmp);
        }
    }
}
void sPrint() {
    cout << "ORDER" << endl;
    cout.width(MAXLEN); cout << "ID";
    cout.width(MAXLEN + 1); cout << "STATUS";
    cout.width(9); cout << "DATE";
    cout.width(MAXLEN + 1); cout << "CUSTOMER`S ID" << endl;

    Order tmp;
    for (int i = 0; i < orders.total; ++i) {
        if (!orders.indT[i].del) {
            dread(orders.tablePtr, &tmp, sizeof(Order), orders.indT[i].ind);
            cout << endl << "--------------------------------------------------------------------------------------------------" << endl;
            edPrint(&tmp);
        }
    }
}
void dbPrint() {
    cout << endl << "==================================================================================================" << endl;

    mPrint();
    cout << endl;
    cout << endl << endl;

    sPrint();
    cout << endl;

    cout << "==================================================================================================" << endl << endl;
}

long binSrch(const forIndTable* indT, const unsigned long key, const long l, const long r) {
    if (l > r) return -1;

    long mid = l + (r - l) / 2;
    if (indT[mid].key == key) return mid;
    if (indT[mid].key > key) return binSrch(indT, key, l, mid - 1);
    return binSrch(indT, key, mid + 1, r);
}
long posSrchInd(const tableInfo* t, const unsigned long key) {
    long pos = binSrch(t->indT, key, 0, t->total - 1);

    if (pos == -1) {
        cout << "error: not found" << endl;
        return -1;
    }
    long pos0 = pos;
    if (t->indT[pos0].del) pos = binSrch(t->indT, key, 0, pos0 - 1);
    if (pos == -1) pos = binSrch(t->indT, key, pos0 + 1, t->total - 1);
    if (pos == -1) {
        cout << "error: not found" << endl;
        return -1;
    }

    return pos;
}
long search(const tableInfo* t, const unsigned long key) {
    long pos = posSrchInd(t, key);
    return pos != -1 ? t->indT[pos].ind : -1;
}
void get_m(char* key) {
    long ind = search(&customers, convert(key));
    if (ind != -1) {
        Customer tmp;
        dread(customers.tablePtr, &tmp, sizeof(Customer), ind);
        pubPrint(&tmp);
    }
}
void get_s(char* key) {
    long ind = search(&orders, convert(key));
    if (ind != -1) {
        Order tmp;
        dread(orders.tablePtr, &tmp, sizeof(Order), ind);
        edPrint(&tmp);
    }
}

long insToInd(const tableInfo* t) {
    long res = t->total;
    for (int i = 0; i < t->total; ++i)
    {
        if (t->indT[i].del) {
            res = t->indT[i].ind;
            for (int j = i + 1; j < t->total; ++j)
            {
                if (!(t->indT[j].del) && (t->indT[j].ind == t->indT[i].ind)) {
                    res = t->total;
                    break;
                }
            }
            if (res == t->indT[i].ind) return res;
        }
    }

    return res;
}
void extend(tableInfo* t) {
    t->capacity += MAXROWS;
    forIndTable* new_indT = (forIndTable*)malloc(t->capacity * sizeof(forIndTable));
    memcpy(new_indT, t->indT, t->total * sizeof(forIndTable));

    free(t->indT);
    t->indT = new_indT;

    fflush(t->tablePtr);

    size_t size = t == &customers ? sizeof(Customer) : sizeof(Order);
    _chsize_s(_fileno(t->tablePtr), size * t->capacity);
}
void updInfIns(tableInfo* t) {
    if (!(t->forDel)) {
        (t->count)++;
        if (t->total < t->count) (t->total)++;
        if (t->total >= t->capacity) extend(t);
    }
    else (t->forDel)--;
}
long insToPos(tableInfo* t, const forIndTable* cell) {
    long i = 0;
    if (!(cell->ind)) {
        t->indT[0] = *cell;
        return 0;
    }
    while (i < t->total && cell->key > t->indT[i].key) i++;
    if (i < t->total && cell->key == t->indT[i].key && !(t->indT[i].del)) return -1;
    if (i == t->total && cell->key == t->indT[i - 1].key && !(t->indT[i].del)) return -1;

    if (i == t->total && cell->key < t->indT[i - 1].key) {
        t->indT[i] = t->indT[i - 1];
        t->indT[i - 1] = *cell;
        return i - 1;
    }

    long res = i;
    forIndTable tmp1, tmp2;
    tmp2 = t->indT[i];
    t->indT[i] = *cell;
    tmp1 = tmp2;

    i++;
    while (i < t->total)
    {
        tmp2 = t->indT[i];
        t->indT[i] = tmp1;
        tmp1 = tmp2;
        i++;
    }
    t->indT[t->total] = tmp1;

    return res;
}
long addNode(const long mPos, const long sPos) {
    long next = customers.indT[mPos].forList;
    customers.indT[mPos].forList = orders.indT[sPos].ind;

    return next;
}
int insert_m(char* info[]) {
    long ind = insToInd(&customers);

    Customer tmp;
    strcpy_s(tmp.id, info[0]);
    strcpy_s(tmp.name, info[1]);
    strcpy_s(tmp.birthday, info[2]);

    unsigned long k = convert(tmp.id);
    forIndTable cell;
    cell.del = 0;
    cell.key = k;
    cell.ind = ind;
    cell.forList = -1;

    long pos = insToPos(&customers, &cell);
    if (pos == -1) {
        cout << "error: the key is already taken" << endl;
        return -1;
    }

    updInfIns(&customers);
    dwrite(customers.tablePtr, &tmp, sizeof(Customer), ind);

    return 0;
}
int insert_s(char* info[]) {

    unsigned long mh = convert(info[3]);
    long mPos = posSrchInd(&customers, mh);
    if (mPos == -1) {
        cout << "error: customer is not found" << endl;
        return -1;
    }

    long ind = insToInd(&orders);

    Order tmp;
    strcpy_s(tmp.id, info[0]);
    strcpy_s(tmp.status, info[1]);
    strcpy_s(tmp.date, info[2]);
    tmp.custID = mh;
    unsigned long k = convert(tmp.id);

    forIndTable cell;
    cell.del = 0;
    cell.key = k;
    cell.ind = ind;
    cell.forList = -1;

    long pos = insToPos(&orders, &cell);
    if (pos == -1) {
        cout << "error: the key is already taken" << endl;
        return -1;
    }

    orders.indT[pos].forList = addNode(mPos, pos);
    updInfIns(&orders);
    dwrite(orders.tablePtr, &tmp, sizeof(Order), ind);

    return 0;
}

void edit_m(char* key, char* newInfo[]) {
    long ind = search(&customers, convert(key));
    Customer tmp;
    dread(customers.tablePtr, &tmp, sizeof(Customer), ind);

    strcpy_s(tmp.name, newInfo[0]);
    strcpy_s(tmp.birthday, newInfo[1]);

    dwrite(customers.tablePtr, &tmp, sizeof(Customer), ind);
}
void edit_s(char* key, char* newInfo[]) {
    long ind = search(&orders, convert(key));
    Order tmp;
    dread(orders.tablePtr, &tmp, sizeof(Order), ind);

    strcpy_s(tmp.status, newInfo[0]);
    strcpy_s(tmp.date, newInfo[1]);

    dwrite(orders.tablePtr, &tmp, sizeof(Order), ind);
}

long srchByInd(tableInfo* t, const long ind)
{
    for (long i = 0; i < t->total; i++)
        if (t->indT[i].ind == ind) return i;
    return -1;
}
void updInfDel(tableInfo* t) {
    (t->forDel)++;
}
int delete_m(char* key) {
    long pos = posSrchInd(&customers, convert(key));
    if (pos == -1) return -1;
    customers.indT[pos].del = 1;
    long next = customers.indT[pos].forList;

    long pos1;
    while (next != -1)
    {
        pos1 = srchByInd(&orders, next);
        orders.indT[pos1].del = 1;
        updInfDel(&orders);
        next = orders.indT[pos1].forList;
    }

    updInfDel(&customers);

    return 0;
}
int delete_s(char* key) {
    long pos = posSrchInd(&orders, convert(key));
    if (pos == -1) return -1;
    orders.indT[pos].del = 1;

    Order tmp;
    dread(orders.tablePtr, &tmp, sizeof(Order), orders.indT[pos].ind);

    long mPos = posSrchInd(&customers, tmp.custID);
    long next = customers.indT[mPos].forList;


    if (next != orders.indT[pos].ind) {
        long pos1;
        while (next != orders.indT[pos].ind)
        {
            pos1 = srchByInd(&orders, next);
            next = orders.indT[pos1].forList;
        }
        orders.indT[pos1].forList = orders.indT[pos].forList;
    }
    else customers.indT[mPos].forList = orders.indT[pos].forList;

    updInfDel(&orders);

    return 0;
}

forIndTable* rewriteIndT(tableInfo* t) {
    forIndTable* newIndT = (forIndTable*)malloc(t->capacity * sizeof(forIndTable));

    int j = 0;
    for (int i = 0; i < t->total; i++)
    {
        if (!(t->indT[i].del)) {
            newIndT[j].del = 0;
            newIndT[j].ind = j;
            newIndT[j].key = t->indT[i].key;
            newIndT[j].forList = -1;

            j++;
        }
    }
    return newIndT;
}
FILE* pubRewrite() {
    int j = 0;
    Customer tmp;
    FILE* fp = dopenTable("mResult.bin", sizeof(Customer));
    for (int i = 0; i < customers.total; i++)
    {
        if (!(customers.indT[i].del)) {
            dread(customers.tablePtr, &tmp, sizeof(Customer), customers.indT[i].ind);
            dwrite(fp, &tmp, sizeof(Customer), j);

            j++;
        }
    }

    return fp;
}
FILE* edRewrite() {
    int j = 0;
    Order tmp;
    FILE* fp = dopenTable("sResult.bin", sizeof(Order));
    for (int i = 0; i < orders.total; i++)
    {
        if (!(orders.indT[i].del)) {
            dread(orders.tablePtr, &tmp, sizeof(Order), orders.indT[i].ind);
            dwrite(fp, &tmp, sizeof(Order), j);

            j++;
        }
    }

    return fp;
}
void restich(forIndTable* mNewInd, forIndTable* sNewInd) {
    Order tmp;
    int j = 0;
    for (int i = 0; i < orders.total; i++)
    {
        if (!(orders.indT[i].del)) {
            dread(orders.tablePtr, &tmp, sizeof(Order), orders.indT[i].ind);

            long mPosNew = binSrch(mNewInd, tmp.custID, 0, customers.total - customers.forDel - 1);
            long sPosNew = binSrch(sNewInd, orders.indT[i].key, 0, j);

            sNewInd[j].forList = mNewInd[mPosNew].forList;
            mNewInd[mPosNew].forList = sNewInd[j].ind;
            j++;
        }
    }

}
void cleaner() {
    forIndTable* mNewInd = rewriteIndT(&customers);
    forIndTable* sNewInd = rewriteIndT(&orders);

    restich(mNewInd, sNewInd);

    FILE* mNewTable = pubRewrite(); // "mResult.bin"
    FILE* sNewTable = edRewrite(); // "sResult.bin"

    free(customers.indT);
    customers.indT = mNewInd;
    free(orders.indT);
    orders.indT = sNewInd;

    customers.total = customers.total - customers.forDel;
    customers.forDel = 0;
    orders.total = orders.total - orders.forDel;
    orders.forDel = 0;

    fclose(customers.tablePtr);
    fclose(mNewTable);
    remove(tm);
    rename("mResult.bin", tm);
    customers.tablePtr = dopenTable(tm, sizeof(Customer));
    fclose(orders.tablePtr);
    fclose(sNewTable);
    remove(ts);
    rename("sResult.bin", ts);
    orders.tablePtr = dopenTable(ts, sizeof(Order));
}
void writeDownInfo(const char* name, tableInfo* t) {
    FILE* fp;
    fopen_s(&fp, name, "wb+");

    dwrite(fp, &t->total, sizeof(long), 0);
    dwrite(fp, &t->forDel, sizeof(long), 1);

    fclose(fp);
}
void writeDownIndT(const char* name, tableInfo* t) {
    FILE* fp;
    fopen_s(&fp, name, "wb+");
    if (fp) {
        for (long i = 0; i < t->total; ++i) dwrite(fp, &t->indT[i], sizeof(forIndTable), i);

        fclose(fp);
        free(t->indT);
    }
}
void dclose() {
    cleaner();

    writeDownInfo(nm, &customers);
    writeDownIndT(im, &customers);
    fclose(customers.tablePtr);

    writeDownInfo(ns, &orders);
    writeDownIndT(is, &orders);
    fclose(orders.tablePtr);
}


void menu() {
    char* info[4];
    for (int i = 0; i < 4; ++i) info[i] = (char*)malloc(MAXLEN * sizeof(char));
    char* key = (char*)malloc(MAXLEN * sizeof(char));

    int option = 1;
    while ((option > 0) && (option < 10)) {
        dopen();
        cout << "Choose an action:" << endl <<
            "1 - print the whole db" << endl <<
            "2 - get-m" << endl << "3 - get-s " << endl <<
            "4 - insert-m" << endl << "5 - insert-s" << endl <<
            "6 - edit-m" << endl << "7 - edit-s" << endl <<
            "8 - delete-m" << endl << "9 - delete-s" << endl <<
            "enter any other character to EXIT" << endl;
        cin >> option;
        cin.ignore();
        switch (option) {
        case 1: // print
            dbPrint();
            break;
        case 2: // get-m
            cout << "Customer's ID: ";
            cin.getline(key, MAXLEN, '\n');

            get_m(key);
            break;
        case 3: // get-s
            cout << "Order's ID: ";
            cin.getline(key, MAXLEN, '\n');

            get_s(key);
            break;
        case 4: // insert-m
            //dopen();
            cout << "Customer's ID: ";
            cin.getline(info[0], MAXLEN, '\n');

            cout << "Name: ";
            cin.getline(info[1], MAXLEN, '\n');

            cout << "Birthday: ";
            cin.getline(info[2], MAXLEN, '\n');

            insert_m(info);
            //dclose();
            break;
        case 5: // insert-s
            //dopen();
            cout << "Order's ID: ";
            cin.getline(info[0], MAXLEN, '\n');

            cout << "Status: ";
            cin.getline(info[1], MAXLEN, '\n');

            cout << "Date(example - 30/12) :";
            cin.getline(info[2], MAXLEN, '\n');

            cout << "Customer's ID: ";
            cin.getline(info[3], MAXLEN, '\n');

            insert_s(info);
            //dclose();
            break;
        case 6: // edit-m
            cout << "Customer's ID: ";
            cin.getline(key, MAXLEN, '\n');

            cout << "New name: ";
            cin.getline(info[0], MAXLEN, '\n');

            cout << "New birthday: ";
            cin.getline(info[1], MAXLEN, '\n');

            edit_m(key, info);
            break;
        case 7:
            cout << "Order's ID: ";
            cin.getline(key, MAXLEN, '\n');

            cout << "New status: ";
            cin.getline(info[0], MAXLEN, '\n');

            cout << "New date: ";
            cin.getline(info[1], MAXLEN, '\n');

            edit_s(key, info);
            break;
        case 8: // delete-m
            cout << "Customer's ID: ";
            cin.getline(key, MAXLEN, '\n');

            delete_m(key);
            break;
        case 9: // delete-s
            cout << "Order's ID: ";
            cin.getline(key, MAXLEN, '\n');

            delete_s(key);
            break;
        default:
            break;
        }
        cout << endl;
        cout << "closing..." << endl;
        dclose();
    }
    for (int i = 0; i < 4; ++i) free(info[i]);
    free(key);
}

int main() {

    //dopen();
    menu();

    //dclose();

    return 0;
}
