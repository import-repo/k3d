# --- L-System Parser/Mutator --- Lj Lapre ----------------------------------
12
5
30
#
c(12)T
#
T=[F~(5)FSd]
S=~(5)FRR[Ba]d>(30)~(5)FRR[Ba]d>(30)!(.8)S
R=[Ba]d>(120)
#
a=~(10)$F[Cxd]~(10)$Fd!(.8)b
b=~(10)$F[Dyd]~(10)$Fd!(.8)a
d=[g(300)g(40)Lg(40)Lg(40)Lg(40)Lg(40)Lg(40)L]
#
B=&B
C=+C
D=-D
#
x=a
y=b
#
F='(1.25)F'(.8)
#
L=[~f(7)c(8){+(30)f(200)-(120)f(200)-(120)f(200)}]
f=z
z=_
@
