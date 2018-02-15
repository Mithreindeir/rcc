# rcc
In development C compiler.
Currently has naive output of a small subset of C to unoptimized three address code

```
int test = 50 / (3 + 5) * 20;
int abcd = test * test + (test / test);
   
if (test == abcd/10) {
	int dcab = test * test;
	abcd = dcab;
} else {
	abcd = test/2;
}
```

int result = test + abcd;

Becomes:

```
.L0:	
	tmp0 := 3 + 5
	tmp1 := 50 / tmp0
	tmp2 := tmp1 * 20
	test = tmp2
	tmp3 := test * test
	tmp4 := test / test
	tmp5 := tmp3 + tmp4
	abcd = tmp5
	tmp6 := abcd / 10
	tmp7 := test == tmp6
	if tmp7 == 0 them goto .L1
	tmp8 := test * test
	dcab = tmp8
	abcd = dcab
	goto .L2
.L1:	
	tmp9 := test / 2
	abcd = tmp9
.L2:	
	tmp10 := test + abcd
	result = tmp10
```