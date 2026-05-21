#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


// MD5 (компактна самодостатня реалізація, RFC 1321)
typedef struct {
    DWORD a, b, c, d;       // стан хешу
    DWORD64 len;            // загальна довжина повідомлення (байт)
    BYTE buffer[64];        // незавершений блок
} MD5_CTX;

// циклічний зсув ліворуч
static DWORD rotl(DWORD x, int n)
{
    return (x << n) | (x >> (32 - n));
}

// чотири логічні функції — по одній на раунд
static DWORD funcF(DWORD b, DWORD c, DWORD d) { return (b & c) | (~b & d); }
static DWORD funcG(DWORD b, DWORD c, DWORD d) { return (b & d) | (c & ~d); }
static DWORD funcH(DWORD b, DWORD c, DWORD d) { return b ^ c ^ d; }
static DWORD funcI(DWORD b, DWORD c, DWORD d) { return c ^ (b | ~d); }

// величини зсуву для кожного з 64 кроків
static const int S[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

// константи k не зберігаються вручну, обчислюються за стандартною формулою
static DWORD K[64];
static bool kReady = false;

static void prepareK()
{
    for (int i = 0; i < 64; i++)
        K[i] = (DWORD)(fabs(sin(i + 1.0)) * 4294967296.0);
    kReady = true;
}

static void md5_block(MD5_CTX* ctx, const BYTE* p)
{
    // розбиття 64 байтів блоку на 16 слів по 4 байти (молодший байт перший)
    DWORD M[16];
    for (int i = 0; i < 16; i++)
        M[i] = (DWORD)p[i * 4]
            | ((DWORD)p[i * 4 + 1] << 8)
            | ((DWORD)p[i * 4 + 2] << 16)
            | ((DWORD)p[i * 4 + 3] << 24);

    DWORD a = ctx->a, b = ctx->b, c = ctx->c, d = ctx->d;

    for (int i = 0; i < 64; i++)
    {
        DWORD f;
        int g;

        if (i < 16)
        {
            f = funcF(b, c, d);
            g = i;
        }
        else if (i < 32)
        {
            f = funcG(b, c, d);
            g = (5 * i + 1) % 16;
        }
        else if (i < 48)
        {
            f = funcH(b, c, d);
            g = (3 * i + 5) % 16;
        }
        else
        {
            f = funcI(b, c, d);
            g = (7 * i) % 16;
        }

        DWORD tmp = d;
        d = c;
        c = b;
        b = b + rotl(a + f + K[i] + M[g], S[i]);
        a = tmp;
    }

    ctx->a += a;
    ctx->b += b;
    ctx->c += c;
    ctx->d += d;
}

static void md5_init(MD5_CTX* ctx)
{
    if (!kReady) prepareK();        // підготовка констант при першому виклику

    ctx->a = 0x67452301;
    ctx->b = 0xefcdab89;
    ctx->c = 0x98badcfe;
    ctx->d = 0x10325476;
    ctx->len = 0;
}

static void md5_update(MD5_CTX* ctx, const BYTE* data, size_t len)
{
    size_t used = (size_t)(ctx->len & 63);
    ctx->len += len;

    if (used)
    {
        size_t need = 64 - used;
        if (len < need)
        {
            memcpy(ctx->buffer + used, data, len);
            return;
        }
        memcpy(ctx->buffer + used, data, need);
        md5_block(ctx, ctx->buffer);
        data += need;
        len -= need;
    }

    while (len >= 64)
    {
        md5_block(ctx, data);
        data += 64;
        len -= 64;
    }

    if (len > 0)
        memcpy(ctx->buffer, data, len);
}

static void md5_final(MD5_CTX* ctx, BYTE out[16])
{
    size_t used = (size_t)(ctx->len & 63);
    DWORD64 bits = ctx->len * 8;

    BYTE pad = 0x80;
    md5_update(ctx, &pad, 1);

    BYTE zero = 0;
    while ((ctx->len & 63) != 56) md5_update(ctx, &zero, 1);

    BYTE lenbytes[8];
    for (int i = 0; i < 8; i++) lenbytes[i] = (BYTE)(bits >> (8 * i));
    md5_update(ctx, lenbytes, 8);

    DWORD st[4] = { ctx->a, ctx->b, ctx->c, ctx->d };
    for (int i = 0; i < 4; i++)
    {
        out[i * 4 + 0] = (BYTE)(st[i]);
        out[i * 4 + 1] = (BYTE)(st[i] >> 8);
        out[i * 4 + 2] = (BYTE)(st[i] >> 16);
        out[i * 4 + 3] = (BYTE)(st[i] >> 24);
    }
}

// обчислення md5 від буфера data довжиною len → out[16]
static void md5(const BYTE* data, size_t len, BYTE out[16])
{
    MD5_CTX ctx;
    md5_init(&ctx);
    md5_update(&ctx, data, len);
    md5_final(&ctx, out);
}


#define SALT_SIZE 8
#define HASH_SIZE 16
#define PWD_MAX   256   // максимальний розмір буфера пароля

static void printSep(const char* title)
{
    printf("\n=========================================================\n");
    printf("%s\n", title);
    printf("=========================================================\n");
}

static void printHex(const char* title, const BYTE* data, size_t len)
{
    printf("%s", title);
    for (size_t i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

// заповнення буфера випадковими байтами (для солі і затирання пам'яті)
static void randomBytes(BYTE* buf, size_t len)
{
    static bool seeded = false;
    if (!seeded)
    {
        srand((unsigned)GetTickCount());    // ініціалізація генератора один раз
        seeded = true;
    }

    for (size_t i = 0; i < len; i++)
        buf[i] = (BYTE)(rand() & 0xFF);
}


static bool isStrongPassword(const char* p)
{
    size_t len = strlen(p);
    bool upper = false, lower = false, digit = false, special = false;

    for (size_t i = 0; i < len; i++)
    {
        unsigned char c = (unsigned char)p[i];
        if (c >= 'A' && c <= 'Z') upper = true;
        else if (c >= 'a' && c <= 'z') lower = true;
        else if (c >= '0' && c <= '9') digit = true;
        else if (c > ' ' && c < 127)   special = true;
    }

    if (len < 8)
    {
        printf("  [-] Пароль закороткий (мінімум 8 символів)\n");
        return false;
    }
    if (!upper)
    {
        printf("  [-] Немає великої літери\n");
        return false;
    }
    if (!lower)
    {
        printf("  [-] Немає малої літери\n");
        return false;
    }
    if (!digit)
    {
        printf("  [-] Немає цифри\n");
        return false;
    }
    if (!special)
    {
        printf("  [-] Немає спецсимволу\n");
        return false;
    }

    return true;
}


static size_t readPasswordSecure(const char* prompt, char* buf, size_t bufSize)
{
    printf("%s", prompt);

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD oldMode = 0;
    GetConsoleMode(hIn, &oldMode);
    SetConsoleMode(hIn, oldMode & ~ENABLE_ECHO_INPUT);   // вимкнути відлуння

    size_t i = 0;
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        if (ch == '\r') continue;
        if (i + 1 < bufSize) buf[i++] = (char)ch;
    }
    buf[i] = '\0';

    SetConsoleMode(hIn, oldMode);                        // відновити режим
    printf("\n");
    return i;
}

static char* allocLockedBuffer(size_t size)
{
    char* buf = (char*)VirtualAlloc(NULL, size,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buf)
    {
        printf("  [ERROR] VirtualAlloc failed: %lu\n", GetLastError());
        return NULL;
    }
    if (!VirtualLock(buf, size))
        printf("  [warn] VirtualLock failed: %lu (буфер не зафіксовано)\n", GetLastError());
    return buf;
}

static void wipeAndFree(char* buf, size_t size)
{
    if (!buf) return;
    SecureZeroMemory(buf, size);            // спочатку нулі
    randomBytes((BYTE*)buf, size);          // потім випадкові байти
    VirtualUnlock(buf, size);               // зняття блокування після затирання
    VirtualFree(buf, 0, MEM_RELEASE);
}

static bool hashPassword(const BYTE salt[SALT_SIZE], const char* pwd, size_t pwdLen, BYTE out[HASH_SIZE])
{
    size_t total = SALT_SIZE + pwdLen;
    char* tmp = allocLockedBuffer(total ? total : 1);
    if (!tmp) return false;
    memcpy(tmp, salt, SALT_SIZE);
    memcpy(tmp + SALT_SIZE, pwd, pwdLen);
    md5((const BYTE*)tmp, total, out);

    wipeAndFree(tmp, total ? total : 1);
    return true;
}


static bool setPassword(BYTE storedHash[HASH_SIZE], BYTE salt[SALT_SIZE])
{
    char* p1 = allocLockedBuffer(PWD_MAX);
    char* p2 = allocLockedBuffer(PWD_MAX);
    if (!p1 || !p2)
    {
        wipeAndFree(p1, PWD_MAX);
        wipeAndFree(p2, PWD_MAX);
        return false;
    }

    bool ok = false;
    size_t len1 = readPasswordSecure("Введіть новий пароль: ", p1, PWD_MAX);
    size_t len2 = readPasswordSecure("Повторіть пароль:     ", p2, PWD_MAX);

    if (len1 != len2 || strcmp(p1, p2) != 0)
        printf("  [-] Паролі не співпадають\n");
    else if (!isStrongPassword(p1))
        printf("  [-] Пароль не відповідає вимогам безпеки\n");
    else
    {
        randomBytes(salt, SALT_SIZE);                 // унікальна сіль

        if (!hashPassword(salt, p1, len1, storedHash)) // зберігаються лише хеш і сіль
        {
            printf("  [ERROR] Не вдалося обчислити геш\n");
        }
        else
        {
            printf("  [+] Пароль встановлено успішно\n");
            ok = true;
        }
    }

    // затирання обох буферів з відкритим паролем після використання
    wipeAndFree(p1, PWD_MAX);
    wipeAndFree(p2, PWD_MAX);
    return ok;
}

static bool checkPassword(const BYTE storedHash[HASH_SIZE], const BYTE salt[SALT_SIZE])
{
    char* p = allocLockedBuffer(PWD_MAX);
    if (!p) return false;

    size_t len = readPasswordSecure("Введіть пароль для перевірки: ", p, PWD_MAX);

    BYTE h[HASH_SIZE];

    if (!hashPassword(salt, p, len, h))
    {
        wipeAndFree(p, PWD_MAX);
        return false;
    }

    wipeAndFree(p, PWD_MAX);                          // пароль більше не потрібен — затирання буфера

    bool match = (memcmp(h, storedHash, HASH_SIZE) == 0);
    SecureZeroMemory(h, HASH_SIZE);
    return match;
}

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    printSep("| Завдання 1 - Пароль, геш MD5 і захист пам'яті      |");

    BYTE storedHash[HASH_SIZE];
    BYTE salt[SALT_SIZE];

    // 1. встановлення пароля
    while (!setPassword(storedHash, salt))
    {
        if (feof(stdin))
        {
            printf("\nВвід завершено передчасно.\n");
            return 1;
        }
        printf("\nСпробуйте ще раз.\n\n");
    }

    printf("\n");
    printHex("Сіль (salt) : ", salt, SALT_SIZE);
    printHex("Геш пароля  : ", storedHash, HASH_SIZE);
    printf("(У пам'яті/на диску зберігаються лише сіль і геш, відкритого пароля немає.)\n\n");

    // 2. перевірка правильним паролем
    printf("\n--- Перевірка №1 (введіть правильний пароль) -\n");
    if (checkPassword(storedHash, salt))
        printf("  >> ДОСТУП ДОЗВОЛЕНО\n\n");
    else
        printf("  >> ВІДМОВА\n\n");

    // 3. перевірка довільним паролем
    printf("--- Перевірка №2 (введіть будь-який пароль) --\n");
    if (checkPassword(storedHash, salt))
        printf("  >> ДОСТУП ДОЗВОЛЕНО\n\n");
    else
        printf("  >> ВІДМОВА\n\n");

    SecureZeroMemory(storedHash, HASH_SIZE);
    SecureZeroMemory(salt, SALT_SIZE);

    printf("Готово.\n");
    return 0;
}
