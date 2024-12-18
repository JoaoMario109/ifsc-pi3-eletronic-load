[Transient Analysis]
{
   Npanes: 6
   {
      traces: 1 {524291,0,"V(Vdut+,Vdut-)"}
      X: ('m',1,0,0.0002,0.00239999999999974)
      Y[0]: (' ',0,-1,1,13)
      Y[1]: ('m',1,1e+308,0.0002,-1e+308)
      Volts: (' ',0,0,0,-1,1,13)
      Log: 0 0 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 1 {524292,0,"V(N002,N003)"}
      X: ('m',1,0,0.0002,0.00239999999999974)
      Y[0]: ('m',0,-0.18,0.02,0.04)
      Y[1]: (' ',0,1e+308,4,-1e+308)
      Volts: ('m',0,0,0,-0.18,0.02,0.04)
      Log: 0 0 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 2 {268959749,0,"V(vdut-)"} {268959756,0,"V(vdut+)"}
      X: ('m',1,0,0.0002,0.00239999999999974)
      Y[0]: (' ',0,5,1,19)
      Y[1]: ('m',1,1e+308,0.0002,-1e+308)
      Volts: (' ',0,0,0,5,1,19)
      Log: 0 0 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 2 {268959750,0,"V(n003)"} {268959751,0,"V(n002)"}
      X: ('m',1,0,0.0002,0.00239999999999974)
      Y[0]: ('m',0,0.06,0.03,0.42)
      Y[1]: (' ',0,1e+308,3,-1e+308)
      Volts: ('m',0,0,0,0.06,0.03,0.42)
      Log: 0 0 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 1 {524290,0,"V(vout)"}
      X: ('m',1,0,0.0002,0.00239999999999974)
      Y[0]: ('m',0,0.15,0.01,0.29)
      Y[1]: (' ',0,1e+308,10,-1e+308)
      Volts: ('m',0,0,0,0.15,0.01,0.29)
      Log: 0 0 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 4 {34603016,0,"I(R3)"} {34603017,0,"I(R1)"} {34603018,0,"I(R2)"} {34603019,0,"I(R4)"}
      X: ('m',1,0,0.0002,0.00239999999999974)
      Y[0]: ('�',1,-5e-006,5e-007,1e-006)
      Y[1]: (' ',0,1e+308,20,-1e+308)
      Amps: ('�',0,0,1,-5e-006,5e-007,1e-006)
      Log: 0 0 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   }
}
[AC Analysis]
{
   Npanes: 6
   {
      traces: 1 {3,0,"V(Vdut+,Vdut-)"}
      X: ('K',1,1,0,10000)
      Y[0]: (' ',3,99.8849369936506,0.002,100.115195553817)
      Y[1]: ('m',1,-0.001,0.0002,0.001)
      Volts: (' ',0,0,0,-1,1,13)
      Log: 1 2 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 1 {4,0,"V(N002,N003)"}
      X: ('K',1,1,0,10000)
      Y[0]: (' ',1,0.575439937337157,0.4,1)
      Y[1]: (' ',0,-32,4,16)
      Volts: ('m',0,0,0,-0.18,0.02,0.04)
      Log: 1 2 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 2 {524293,0,"V(vdut-)"} {524300,0,"V(vdut+)"}
      X: ('K',1,1,0,10000)
      Y[0]: (' ',0,44.6683592150963,1,158.489319246111)
      Y[1]: ('m',1,-0.001,0.0002,0.0012)
      Volts: (' ',0,0,0,5,1,19)
      Log: 1 2 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 2 {524294,0,"V(n003)"} {524295,0,"V(n002)"}
      X: ('K',1,1,0,10000)
      Y[0]: (' ',1,0.691830970918936,0.8,2.08929613085404)
      Y[1]: (' ',0,-36,3,-0)
      Volts: ('m',0,0,0,0.06,0.03,0.42)
      Log: 1 2 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 1 {524290,0,"V(vout)"}
      X: ('K',1,1,0,10000)
      Y[0]: (' ',0,0.0223872113856834,3,1.99526231496888)
      Y[1]: (' ',0,-130,10,-0)
      Volts: ('m',0,0,0,0.15,0.01,0.29)
      Log: 1 2 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   },
   {
      traces: 4 {34603016,0,"I(R3)"} {34603017,0,"I(R1)"} {34603018,0,"I(R2)"} {34603019,0,"I(R4)"}
      X: ('K',1,1,0,10000)
      Y[0]: (' ',0,3.98107170553497e-006,2,5.01187233627272e-005)
      Y[1]: (' ',0,-40,20,180)
      Amps: ('�',0,0,1,-5e-006,5e-007,1e-006)
      Log: 1 2 0
      GridStyle: 1
      PltMag: 1
      PltPhi: 1 0
   }
}
