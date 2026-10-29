"""Analyze one timestamped capture window. No default mains calibration."""
import argparse
import bisect
import csv
import json
import math
from pathlib import Path


def analyze(rows, voltage_scale=1.0, current_scale=1.0):
    if len(rows) < 16:
        raise ValueError("At least 16 sample pairs are required")
    if not all(math.isfinite(x) and x > 0 for x in (voltage_scale, current_scale)):
        raise ValueError("Calibration scales must be finite and positive")
    data = [[float(row[k]) for k in ("t_v_us", "sensor_v", "t_i_us", "sensor_i")]
            for row in rows]
    if not all(math.isfinite(x) for row in data for x in row):
        raise ValueError("Capture contains non-finite values")
    tv, v, ti, i = map(list, zip(*data))
    if any(b <= a for times in (tv, ti) for a, b in zip(times, times[1:])):
        raise ValueError("Each channel's timestamps must strictly increase; use one window")
    if min(tv + ti) < 0:
        raise ValueError("Timestamps must be nonnegative")
    if min(v+i) < 0 or max(v+i) >= 3.25:
        raise ValueError("Input near/outside ADC bench range; check bias and clipping")
    # Resample current onto voltage times. This mitigates skew, not analog phase error.
    paired=[]
    for t, volts in zip(tv, v):
        j=bisect.bisect_left(ti,t)
        if j == 0 or j == len(ti):
            continue
        f=(t-ti[j-1])/(ti[j]-ti[j-1])
        paired.append((t,volts,i[j-1]+f*(i[j]-i[j-1])))
    if len(paired)<16: raise ValueError("Insufficient overlapping channel samples")
    ts,vs,ins=map(list,zip(*paired))
    vm=sum(vs)/len(vs); im=sum(ins)/len(ins)
    vs=[(x-vm)*voltage_scale for x in vs]
    ins=[(x-im)*current_scale for x in ins]
    vrms=math.sqrt(sum(x*x for x in vs)/len(vs))
    irms=math.sqrt(sum(x*x for x in ins)/len(ins))
    p=sum(a*b for a,b in zip(vs,ins))/len(vs)
    apparent=vrms*irms
    crossings=[]
    for j in range(1,len(vs)):
        if vs[j-1] < 0 <= vs[j]:
            f=-vs[j-1]/(vs[j]-vs[j-1])
            crossings.append(ts[j-1]+f*(ts[j]-ts[j-1]))
    frequency=(len(crossings)-1)*1e6/(crossings[-1]-crossings[0]) if len(crossings)>1 else None
    return {"samples":len(vs), "duration_s":(ts[-1]-ts[0])/1e6,
            "voltage_rms_scaled":vrms,"current_rms_scaled":irms,
            "active_power_scaled":p,"apparent_power_scaled":apparent,
            "power_factor_estimate":p/apparent if apparent>1e-12 else None,
            "frequency_hz":frequency,
            "voltage_crest_factor":max(map(abs,vs))/vrms if vrms>1e-12 else None,
            "voltage_scale":voltage_scale,"current_scale":current_scale,
            "note":"Sequential ADC estimates; no certified power-quality accuracy claimed"}


def load(path):
    lines=[s for s in Path(path).read_text().splitlines() if s and not s.startswith('#')]
    return list(csv.DictReader(lines))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('capture',help='CSV containing exactly one firmware capture window')
    p.add_argument('--voltage-scale',type=float,default=1)
    p.add_argument('--current-scale',type=float,default=1)
    args=p.parse_args()
    try: result=analyze(load(args.capture),args.voltage_scale,args.current_scale)
    except (ValueError,KeyError) as e: p.error(str(e))
    print(json.dumps(result,indent=2,allow_nan=False))

if __name__=='__main__': main()
