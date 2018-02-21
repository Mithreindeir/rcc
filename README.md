# rcc
In development C compiler.
Currently has naive output of a small subset of C to unoptimized three address code

```
int pow(int num, int p)
{
	int result = 1;
	for (int i = 0; i < p; i++)
		result = result * num;
	return result;
}

int main(int argc, char **argv)
{
	for (int i = 0; i < 10; i++) {
		int num = pow(i, 2);
	}
	return 0;
}


```

Becomes:

```
pow:
	result = 1
	i = 0
	goto   L1
L0
	tmp0 = result*num
	result = tmp0
L2
	tmp1 = i
	i = i+1
L1
	jge i p L3
	goto   L0
L3
	ret result

main:
	i = 0
	goto   L5
L4
	param i
	param 2
	num = pow()
L6
	tmp3 = i
	i = i+1
L5
	jge i 10 L7
	goto   L4
L7
	ret 0


```
