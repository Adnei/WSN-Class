// RSA Implementation from
//      https://www.techiedelight.com/rsa-algorithm-implementation-c/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define RAND_CAP 100
#define PRECISION 6

int e, d, n;

int FastExponention(int bit, int n, int* y, int* a)
{
    if (bit == 1) {
        *y = (*y * (*a)) % n;
    }

    *a = (*a) * (*a) % n;
}

int FindT(int a, int m, int n)
{
    int r;
    int y = 1;

    while (m > 0)
    {
        r = m % 2;
        FastExponention(r, n, &y, &a);
        m = m / 2;
    }
    return y;
}

int gcd(int a, int b)
{
    int q, r1, r2, r;

    if (a > b)
    {
        r1 = a;
        r2 = b;
    }
    else {
        r1 = b;
        r2 = a;
    }

    while (r2 > 0)
    {
        q = r1 / r2;
        r = r1 - q * r2;
        r1 = r2;
        r2 = r;
    }

    return r1;
}

int PrimarityTest(int a, int i)
{
    int n = i - 1;
    int k = 0;
    int j, m, T;

    while (n % 2 == 0)
    {
        k++;
        n = n / 2;
    }

    m = n;
    T = FindT(a, m, i);

    if (T == 1 || T == i - 1) {
        return 1;
    }

    for (j = 0; j < k; j++)
    {
        T = FindT(T, 2, i);
        if (T == 1) {
            return 0;
        }
        if (T == i - 1) {
            return 1;
        }
    }
    return 0;
}


// int PrimarityTest(int from, int num){
//   for(int i = from; i < PRECISION; i++){
//     if ( ((int) pow(i, num-1) % num) != -1 ){
//       return 0;
//     }
//   }
//   return 1;
// }

int inverse(int a, int b)
{
    int inv;
    int q, r, r1 = a, r2 = b, t, t1 = 0, t2 = 1;

    while (r2 > 0)
    {
        q = r1 / r2;
        r = r1 - q * r2;
        r1 = r2;
        r2 = r;

        t = t1 - q * t2;
        t1 = t2;
        t2 = t;
    }

    if (r1 == 1) {
        inv = t1;
    }

    if (inv < 0) {
        inv = inv + a;
    }

    return inv;
}

int KeyGeneration()
{
    int p, q;
    int phi_n;

    do {
        do
            p = rand() % RAND_CAP + 1;
        while (p % 2 == 0);
    } while (!PrimarityTest(2, p));

    do {
        do
            q = rand() % RAND_CAP + 1;
        while (q % 2 == 0);
    } while (!PrimarityTest(2, q));

    n = p * q;
    phi_n = (p - 1) * (q - 1);

    do
        e = rand() % (phi_n - 2) + 2; // 1 < e < phi_n
    while (gcd(e, phi_n) != 1);

    d = inverse(phi_n, e);
}

// int EncryptDecrypt(int key, int value){
//   return FindT(value, key, n);
// }


double EncryptDecrypt(int key, int value){
  double pow_value = pow(value, key);
  printf("pow_value = %f\n", pow_value);
  double mod_result = fmod(pow_value, n);
  printf("mod_result = %f\n",mod_result);
  return mod_result;
}

int main(void){
  char my_str[2] = "Hi";
  srand(time(0));
  KeyGeneration();
  printf("private = %d; N = %d; e = %d;\n", d, n, e);
  for(int i = 0; i < strlen(my_str); i++ ){
    int plain = (int) my_str[i];
    printf("char = %c, AscII = %d\n",my_str[i], (int) my_str[i]);
    int cipher_value = EncryptDecrypt(e, plain);
    printf("Cipher = %d\n", cipher_value);
    int mixed = cipher_value;
    int decipher_value = EncryptDecrypt(d, mixed);
    printf("Decipher = %c\n", (char) decipher_value);
  }
  return 0;

}
