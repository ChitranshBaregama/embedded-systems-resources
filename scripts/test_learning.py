import contextlib
import copy
import datetime as dt
import io
from pathlib import Path
import tempfile
import unittest
import learning

class LearningTests(unittest.TestCase):
    def row(self,**changes):
        r=dict(zip(learning.FIELDS,['A00001','2026-01-01','E0001','first','Solved without help','medium','30','0','no','yes','pass','3','3','3','test evidence']))
        r.update(changes);return r

    def test_valid_unassisted_and_assisted(self):
        learning.check_rows([self.row()],{'E0001'})
        learning.check_rows([self.row(status='Solved',hints='1',assisted='yes')],{'E0001'})

    def test_cannot_claim_independence_with_hints_or_exposure(self):
        for change in ({'hints':'1'},{'assisted':'yes'},{'correct':'no'},{'warnings':'fail'}):
            with self.subTest(change=change),self.assertRaises(ValueError): learning.check_rows([self.row(**change)],{'E0001'})

    def test_reject_bad_values(self):
        for change in ({'item_id':'../bad'},{'minutes':'0'},{'hints':'-1'},{'explanation':'5'},{'evidence':''},{'status':'Unseen'},{'date':'not-a-date'}):
            with self.subTest(change=change),self.assertRaises(ValueError):learning.check_rows([self.row(**change)],{'E0001'})

    def test_duplicate_first_and_id(self):
        first=self.row()
        with self.assertRaises(ValueError): learning.check_rows([first,first],{'E0001'})
        with self.assertRaises(ValueError): learning.check_rows([first,self.row(attempt_id='A00002',date='2026-01-02')],{'E0001'})

    def test_retention_requires_anchor_and_interval(self):
        retest=self.row(attempt_id='A00002',kind='retention7',date='2026-01-07')
        with self.assertRaises(ValueError):learning.check_rows([retest],{'E0001'})
        with self.assertRaises(ValueError):learning.check_rows([self.row(),retest],{'E0001'})
        retest['date']='2026-01-08';learning.check_rows([self.row(),retest],{'E0001'})

    def test_ready_requires_both_retention_checks(self):
        ready=self.row(attempt_id='A00004',kind='review',status='Interview Ready',date='2026-02-02')
        with self.assertRaises(ValueError):learning.check_rows([self.row(),ready],{'E0001'})
        rows=[self.row(),self.row(attempt_id='A00002',kind='retention7',date='2026-01-08'),self.row(attempt_id='A00003',kind='retention30',date='2026-01-31'),ready]
        learning.check_rows(rows,{'E0001'})
        rows[2]['assisted']='yes';rows[2]['status']='Solved'
        with self.assertRaises(ValueError):learning.check_rows(rows,{'E0001'})

    def test_new_paths_and_no_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);(root/'practice/questions').mkdir(parents=True);(root/'practice/progress').mkdir()
            (root/'practice/progress/catalog.csv').write_text('id,prompt\nE0001,Add with a checked contract\n')
            for item,slug in [('E0001','../escape'),('E1501','test'),('Q0000','test'),('../Q0001','test')]:
                with self.assertRaises(ValueError):learning.new_question(root,item,slug)
            p=learning.new_question(root,'E0001','addition')
            self.assertIn('Deliberately empty',(p/'solution.c').read_text())
            self.assertIn('#error',(p/'test.c').read_text())
            with self.assertRaises(ValueError):learning.new_question(root,'E0001','different_slug')

    def test_actual_repository_invariants(self):
        self.assertIn('1500 preserved drills',learning.validate())

if __name__=='__main__':unittest.main()
