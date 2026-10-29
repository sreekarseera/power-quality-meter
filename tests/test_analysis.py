import math
import unittest
from tools.analyze import analyze


def signal(phase=0, frequency=50):
    return [dict(t_v_us=n*2500,sensor_v=1.65+.3*math.sin(2*math.pi*frequency*n*.0025),
                 t_i_us=n*2500+1000,sensor_i=1.65+.2*math.sin(2*math.pi*frequency*(n*.0025+.001)-phase))
            for n in range(401)]

class AnalysisTests(unittest.TestCase):
    def test_sine_rms_frequency_pf(self):
        r=analyze(signal(),10,5)
        self.assertAlmostEqual(r['voltage_rms_scaled'],3/math.sqrt(2),delta=.015)
        self.assertAlmostEqual(r['frequency_hz'],50,delta=.05)
        self.assertAlmostEqual(r['power_factor_estimate'],1,delta=.003)
    def test_phase_shift(self):
        r=analyze(signal(math.pi/3))
        self.assertAlmostEqual(r['power_factor_estimate'],.5,delta=.015)
    def test_constant_has_no_power_or_frequency(self):
        rows=signal()
        for row in rows: row['sensor_v']=row['sensor_i']=1.65
        r=analyze(rows)
        self.assertIsNone(r['power_factor_estimate'])
        self.assertIsNone(r['frequency_hz'])
    def test_bad_captures_rejected(self):
        for problem in ['duplicate','nan','clipping']:
            rows=signal()
            if problem=='duplicate': rows[1]['t_v_us']=rows[0]['t_v_us']
            if problem=='nan': rows[0]['sensor_v']=float('nan')
            if problem=='clipping': rows[0]['sensor_i']=3.3
            with self.assertRaises(ValueError): analyze(rows)

if __name__=='__main__':unittest.main()
