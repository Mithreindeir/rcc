# rcc
In development C compiler.
Currently has naive output of a small subset of C to unoptimized three address code

```
int main(int argc, char **argv)
{
	char arg = 0;
	for (int i = 0; i < argc; i++) {
		if (argv[i][0] == '-') { 
			arg = argv[i][1];
			break;
		}
	}
}

```

Becomes:

```
	arg = 0
	i = 0
	goto   L1
L0
	tmp0 = i*4
	tmp1 = argv+tmp0
	tmp2 = 0*4
	tmp3 = *tmp1+tmp2
	jneq *tmp3 45 L3
	goto   L4
L4
	tmp5 = i*4
	tmp6 = argv+tmp5
	tmp7 = 1*4
	tmp8 = *tmp6+tmp7
	arg = *tmp8
	goto   L5
L3
L2
	tmp9 = i
	i = i+1
L1
	jge i argc L5
	goto   L0
L5

```
