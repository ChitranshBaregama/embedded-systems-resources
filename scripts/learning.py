#!/usr/bin/env python3
"""Learning infrastructure only. Never supplies or scores a learner implementation."""
import argparse
import csv
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import re
import statistics
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
FIELDS = 'attempt_id,date,item_id,kind,status,category,minutes,hints,assisted,correct,warnings,debugging,test_quality,explanation,evidence'.split(',')
STATES = ['Unseen', 'Attempted', 'Solved', 'Solved without help', 'Interview Ready', 'Revisit']
KINDS = ['first', 'retention7', 'retention30', 'review']

def read_csv(path):
    with path.open(encoding='utf-8', newline='') as f:
        return list(csv.DictReader(f))

def ledger(root=ROOT):
    p = root / 'practice/progress/attempts.csv'
    with p.open(encoding='utf-8', newline='') as f:
        reader = csv.DictReader(f)
        if reader.fieldnames != FIELDS:
            raise ValueError('Attempt ledger schema mismatch')
        return list(reader)

def item_ids(root=ROOT):
    result = {r['id'] for r in read_csv(root/'practice/progress/catalog.csv')}
    result.update(f'I{i:03d}' for i in range(1, 151))
    result.update(p.name.split('_', 1)[0] for p in (root/'practice/questions').iterdir()
                  if p.is_dir() and re.fullmatch(r'[EQ]\d{4}_[a-z0-9_]+', p.name))
    return result

def independent(row):
    return row['correct'] == 'yes' and row['assisted'] == 'no' and int(row['hints']) == 0

def check_rows(rows, known):
    seen = set()
    prior = []
    last_date = None
    for row in rows:
        if set(row) != set(FIELDS) or any(v is None for v in row.values()):
            raise ValueError('Malformed attempt row')
        if not re.fullmatch(r'A\d{5,}', row['attempt_id']) or row['attempt_id'] in seen:
            raise ValueError('Invalid or duplicate attempt ID')
        seen.add(row['attempt_id'])
        date = dt.date.fromisoformat(row['date'])
        if last_date is not None and date < last_date:
            raise ValueError('Ledger dates must be nondecreasing; append corrections as review notes')
        last_date = date
        if row['item_id'] not in known:
            raise ValueError('Unknown item ID: '+row['item_id'])
        if row['kind'] not in KINDS or row['status'] not in STATES:
            raise ValueError('Unknown kind/status')
        if row['category'] not in ('quick', 'medium', 'deep', 'oral'):
            raise ValueError('Unknown category')
        if not 1 <= int(row['minutes']) <= 1440 or int(row['hints']) < 0:
            raise ValueError('Invalid time or hint count')
        if row['assisted'] not in ('yes', 'no') or row['correct'] not in ('yes', 'no'):
            raise ValueError('Correct/assisted must be yes or no')
        if row['warnings'] not in ('pass', 'fail', 'na'):
            raise ValueError('Invalid warnings result')
        if any(not 0 <= int(row[k]) <= 4 for k in ('debugging', 'test_quality', 'explanation')):
            raise ValueError('Scores must be 0..4')
        if not row['evidence'].strip():
            raise ValueError('Evidence note or link required')
        if row['status'] == 'Unseen':
            raise ValueError('Unseen is implicit; do not create fake attempts')
        if row['status'] in ('Solved', 'Solved without help', 'Interview Ready'):
            if row['correct'] != 'yes' or row['warnings'] == 'fail':
                raise ValueError('Solved states require correctness and no unresolved compiler warnings')
        if row['status'] in ('Solved without help', 'Interview Ready') and not independent(row):
            raise ValueError('Independent state cannot contain hints or assistance')
        earlier = [r for r in prior if r['item_id'] == row['item_id']]
        if row['kind'] == 'first' and any(r['kind'] == 'first' for r in earlier):
            raise ValueError('Only one first attempt per item; use review for later work')
        if row['kind'].startswith('retention'):
            successes = [r for r in earlier if r['correct'] == 'yes' and r['kind'] in ('first','review')]
            if not successes:
                raise ValueError('Retention needs a previous correct attempt')
            anchor = min(dt.date.fromisoformat(r['date']) for r in successes)
            days = 7 if row['kind'] == 'retention7' else 30
            if (date-anchor).days < days:
                raise ValueError('Retention interval not reached')
        if row['status'] == 'Interview Ready':
            if row['kind'] != 'review' or any(int(row[k]) < 3 for k in ('debugging','test_quality','explanation')):
                raise ValueError('Interview Ready requires a scored reviewer decision')
            if not all(any(r['kind']==k and independent(r) for r in earlier) for k in ('retention7','retention30')):
                raise ValueError('Interview Ready requires independent 7- and 30-day evidence')
        prior.append(row)

def validate(root=ROOT):
    catalog = read_csv(root/'practice/progress/catalog.csv')
    expected = [f'E{i:04d}' for i in range(1, 1501)]
    if [r['id'] for r in catalog] != expected:
        raise ValueError('Catalog must contain ordered E0001..E1500 exactly once')
    bank = (root/'practice/embedded-c-1500-exercises.md').read_text(encoding='utf-8')
    prompts = {int(m[1]):m[2] for m in re.finditer(r'^(\d+)\. (.+)$',bank,re.M)}
    if any(r['prompt'] != prompts.get(int(r['number'])) for r in catalog):
        raise ValueError('Catalog differs from preserved source prompts')
    interview = (root/'practice/interview-150/README.md').read_text(encoding='utf-8')
    if re.findall(r'^### (I\d{3})$',interview,re.M) != [f'I{i:03d}' for i in range(1,151)]:
        raise ValueError('Interview IDs must be I001..I150 exactly once')
    source = json.loads((root/'career/migration/Firmware-Interview-Prep.snapshot.json').read_text(encoding='utf-8'))
    for f in source['files']:
        data = f['content'].encode('utf-8')
        sha = hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()
        if sha != f['sha']:
            raise ValueError('Source preservation hash mismatch: '+f['path'])
    known = item_ids(root)
    folders = list((root/'practice/questions').glob('[EQ][0-9][0-9][0-9][0-9]_*'))
    ids = [p.name.split('_',1)[0] for p in folders]
    if len(ids) != len(set(ids)):
        raise ValueError('Duplicate practice directory ID')
    for p in folders:
        if not all((p/f).is_file() for f in ('question.md','solution.c','test.c','notes.md')):
            raise ValueError('Incomplete scaffold: '+p.name)
    check_rows(ledger(root),known)
    return f'PASS: 1500 preserved drills, 150 interview prompts, {len(source["files"])} source blobs, {len(folders)} scaffolds; ledger valid'

def new_question(root, item, slug):
    if not re.fullmatch(r'[EQ]\d{4}',item) or item[1:] == '0000':
        raise ValueError('Use E0001..E1500 or a nonzero QXXXX ID')
    if item.startswith('E') and int(item[1:]) > 1500:
        raise ValueError('E IDs are reserved for the preserved 1500 bank')
    if not re.fullmatch(r'[a-z][a-z0-9_]{0,59}',slug):
        raise ValueError('Slug must be lowercase letters/digits/underscores, beginning with a letter')
    base = root/'practice/questions'
    if list(base.glob(item+'_*')):
        raise ValueError('ID already has a folder; never overwrite a learner attempt')
    target = base/(item+'_'+slug)
    target.mkdir()
    prompt = next((r['prompt'] for r in read_csv(root/'practice/progress/catalog.csv') if r['id']==item),'Define the precise problem before attempting.')
    files = {
        'question.md':f'# {item}: {slug}\n\n{prompt}\n\n## Contract before scoring\n\nAgree input domain, overflow, invalid-input behavior, ownership, target assumptions, complexity, test oracle and timebox.\n',
        'solution.c':'/* Learner implementation only. Deliberately empty. */\n',
        'test.c':'#error "Define meaningful contract tests before scoring this exercise."\n',
        'notes.md':'# Attempt notes\n\nStatus: Unseen\n\nApproach, mistakes, complexity, evidence and retention: not yet recorded.\n'}
    for name,text in files.items(): (target/name).write_text(text,encoding='utf-8')
    return target

def run_question(root,item):
    if not re.fullmatch(r'[EQ]\d{4}',item): raise ValueError('Invalid question ID')
    matches=list((root/'practice/questions').glob(item+'_*'))
    if len(matches)!=1: raise ValueError('Question must have exactly one prepared folder')
    folder=matches[0]
    out=root/'.learning-build'; out.mkdir(exist_ok=True)
    exe=out/(item+('.exe' if os.name=='nt' else ''))
    # CC is one executable path, not a shell command; subprocess never uses a shell.
    command=[os.environ.get('CC','gcc'),'-std=c11','-Wall','-Wextra','-Werror','-Wconversion','-Wshadow','-Wpedantic','-g',str(folder/'solution.c'),str(folder/'test.c'),'-o',str(exe)]
    subprocess.run(command,check=True)
    subprocess.run([str(exe)],check=True)
    print('Harness passed. No progress or readiness was changed; record the reviewed outcome separately.')

def summary(root,today):
    rows=ledger(root);check_rows(rows,item_ids(root))
    visible=[r for r in rows if dt.date.fromisoformat(r['date'])<=today]
    first=[r for r in visible if r['kind']=='first']
    print(f'As of {today}: {len(first)} scored first attempts; {sum(independent(r) for r in first)} independent successes. Unrecorded items remain Unseen.')
    for category in ('quick','medium','deep','oral'):
        selected=[r for r in first if r['category']==category]
        if selected: print(f'{category}: independent {sum(independent(r) for r in selected)}/{len(selected)}, median {statistics.median(int(r["minutes"]) for r in selected)} minutes')
    anchors={}
    for r in visible:
        if r['correct']=='yes' and r['kind'] in ('first','review'):
            anchors.setdefault(r['item_id'],dt.date.fromisoformat(r['date']))
    for item,anchor in sorted(anchors.items()):
        for days in (7,30):
            done=any(r['item_id']==item and r['kind']==f'retention{days}' and independent(r) for r in visible)
            due=anchor+dt.timedelta(days=days)
            if not done and due<=today: print(f'DUE {item}: {days}-day retention (due {due})')
    print('Gate status requires reviewer evidence; mocks/projects are tracked in their own evidence tables.')

def main():
    p=argparse.ArgumentParser(description=__doc__); sub=p.add_subparsers(dest='command',required=True)
    sub.add_parser('validate')
    s=sub.add_parser('summary');s.add_argument('--today',type=dt.date.fromisoformat,default=dt.date.today())
    n=sub.add_parser('new');n.add_argument('item');n.add_argument('slug')
    r=sub.add_parser('run');r.add_argument('item')
    log=sub.add_parser('log')
    log.add_argument('item');log.add_argument('--date',type=dt.date.fromisoformat,default=dt.date.today())
    log.add_argument('--kind',choices=KINDS,required=True);log.add_argument('--status',choices=STATES,required=True)
    log.add_argument('--category',choices=['quick','medium','deep','oral'],required=True)
    for key in ('minutes','hints','debugging','test-quality','explanation'): log.add_argument('--'+key,type=int,required=True)
    for key in ('assisted','correct'): log.add_argument('--'+key,choices=['yes','no'],required=True)
    log.add_argument('--warnings',choices=['pass','fail','na'],required=True)
    log.add_argument('--evidence',required=True)
    a=p.parse_args()
    try:
        if a.command=='validate': print(validate())
        elif a.command=='summary': summary(ROOT,a.today)
        elif a.command=='new': print(new_question(ROOT,a.item,a.slug))
        elif a.command=='run': run_question(ROOT,a.item)
        elif a.command=='log':
            rows=ledger();check_rows(rows,item_ids())
            if a.date>dt.date.today(): raise ValueError('Do not log future attempts')
            row={k:str(getattr(a,k,'')) for k in FIELDS}
            row.update(attempt_id=f'A{max([int(r["attempt_id"][1:]) for r in rows]+[0])+1:05d}',item_id=a.item,date=a.date.isoformat())
            check_rows(rows+[row],item_ids())
            path=ROOT/'practice/progress/attempts.csv';tmp=path.with_suffix('.csv.tmp')
            with tmp.open('w',encoding='utf-8',newline='') as f:
                writer=csv.DictWriter(f,fieldnames=FIELDS);writer.writeheader();writer.writerows(rows+[row])
            tmp.replace(path)
            print('Recorded '+row['attempt_id']+'; review and commit the ledger change. Single-writer CLI: do not run concurrent log commands.')
    except (ValueError,OSError,subprocess.CalledProcessError) as e:
        print('ERROR: '+str(e),file=sys.stderr);return 1
    return 0

if __name__=='__main__': sys.exit(main())
