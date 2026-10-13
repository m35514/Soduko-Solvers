"""Reproduce the report from preserved source measurements; never rewrites inputs."""
from pathlib import Path
import sys, json, hashlib, subprocess, os, platform
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tmp/sudoku_analysis_deps'))
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from scipy import stats

OUT=ROOT/'output/analysis'
OUT.mkdir(parents=True,exist_ok=True)
ASSETS=OUT/'figures'
ASSETS.mkdir(exist_ok=True)
NAMED=Path(r'C:\Users\malob\.codex\attachments\3cfd426d-cc26-43c5-aea5-cee45793a8e3\Pasted text.txt')
NOTEBOOK=Path(r'C:\Users\malob\.codex\attachments\dc0feb9f-84ae-4f7f-ba77-8d7bd03c948e\Pasted text.txt')
PRIMARY=ROOT/'results/master_collection_benchmark.csv'
HISTORY=ROOT/'results/master_collection_metrics.csv'
SOLVERS=['Naive','Bitmask','Solver3_bitmask','Solver3_array','Solver4_bitmask','Solver4_array','Solver5_bitmask','Solver5_array']
MAIN=['Naive','Bitmask','Solver3_bitmask','Solver4_bitmask','Solver5_bitmask']
SHORT={'Naive':'S1 naive','Bitmask':'S2 bitmask',**{f'Solver{i}_{r}':f'S{i} {r}' for i in [3,4,5] for r in ['array','bitmask']}}
COLORS=['#52606d','#3b82a0','#16836a','#55a98b','#bd742b','#db9d50','#7955a1','#a48bc0']
METRICS=['Time (ms)','Samples','Max depth','Guesses','Dead ends','Placements','Eliminations','Events']

def load(path):
    d=pd.read_csv(path,dtype={'Puzzle String':str})
    d['Solver']=d.Solver.str.replace(' ','_',regex=False)
    assert set(d.Solver)==set(SOLVERS)
    assert d['Puzzle String'].str.fullmatch('[0-9]{81}').all()
    assert not d.duplicated(['Puzzle','Solver']).any()
    assert (d.groupby('Puzzle').Solver.nunique()==8).all()
    for m in METRICS:d[m]=pd.to_numeric(d[m],errors='coerce')
    d['clues']=d['Puzzle String'].map(lambda s:sum(c!='0' for c in s))
    d['collection']=d.Puzzle.str.rsplit(':',n=1).str[0]
    return d

def pivot(d,m):
    return d.pivot_table(index='Puzzle String',columns='Solver',values=m,aggfunc='median').reindex(columns=SOLVERS)

def fmt(x,n=3):
    return 'n.a.' if pd.isna(x) else f'{x:,.{n}f}'

def table(headers,rows):
    return '\n'.join(['| '+' | '.join(headers)+' |','| '+' | '.join(['---']*len(headers))+' |']+['| '+' | '.join(map(str,r))+' |' for r in rows])+'\n'

def savefig(name):
    plt.savefig(ASSETS/(name+'.png'),dpi=190,bbox_inches='tight',facecolor='white')
    plt.savefig(ASSETS/(name+'.svg'),bbox_inches='tight',facecolor='white')
    plt.close()

def independent_valid(puzzle,solution):
    if len(solution)!=81 or not set(solution)<=set('123456789'):return False
    if any(a!='0' and a!=b for a,b in zip(puzzle,solution)):return False
    b=np.array(list(solution)).reshape(9,9)
    target=set('123456789')
    return all(set(b[i,:])==target and set(b[:,i])==target for i in range(9)) and all(set(b[r:r+3,c:c+3].flat)==target for r in [0,3,6] for c in [0,3,6])

d=load(PRIMARY); named=load(NAMED)
t=pivot(d,'Time (ms)').dropna(); nt=pivot(named,'Time (ms)').dropna()
assert (t>0).all().all() and (nt>0).all().all()
g=pivot(d,'Guesses'); ng=pivot(named,'Guesses')
gc=g.dropna(); common=gc.index.intersection(t.index)
gc=gc.loc[common]; tc=t.loc[common]
assert (gc.Naive==gc.Bitmask).all()
for level in [3,4,5]:
    assert (g[f'Solver{level}_array']==g[f'Solver{level}_bitmask']).all()
for metric in ['Guesses','Dead ends','Placements','Eliminations','Events']:
    assert (d.groupby(['Puzzle String','Solver'])[metric].nunique()<=1).all()
history=pd.read_csv(HISTORY,dtype={'puzzle':str,'final_board':str})
history['key']=history.collection+':'+history.source_line.astype(str)
j=d.merge(history,left_on=['Puzzle','Solver'],right_on=['key','solver'],validate='one_to_one')
assert len(j)==len(d) and (j['Puzzle String']==j.puzzle).all()
node_match=int((j.Guesses==j.nodes).sum())
assert node_match==int(j.Guesses.notna().sum())
stored_valid=sum(independent_valid(p,s) for p,s in zip(history.puzzle,history.final_board))
assert stored_valid==len(history)
assert (history.groupby(['collection','source_line']).final_board.nunique()==1).all()

# Freeze a small, inexpensive fixture set before fresh execution.
names=d.drop_duplicates('Puzzle String').set_index('Puzzle String').Puzzle
eligible=gc.index[(gc.Naive<15000)&(gc.Naive>0)]
q=gc.loc[eligible]
cases={}
cases['zero_guess']=q.index[(q.Solver3_bitmask==0)&(q.Naive>100)][0]
for category,a,b in [('naked_pair_help','Solver3_bitmask','Solver4_bitmask'),('xwing_help','Solver4_bitmask','Solver5_bitmask')]:
    ix=q.index[(q[b]<q[a])&(t.loc[q.index,b]<t.loc[q.index,a])]
    ratio=t.loc[ix,a]/t.loc[ix,b]
    cases[category]=ratio.idxmax()
ix=q.index[(q.Solver3_bitmask==q.Solver4_bitmask)&(q.Solver4_bitmask==q.Solver5_bitmask)&(q.Solver3_bitmask>0)&(t.loc[q.index,'Solver5_bitmask']>t.loc[q.index,'Solver3_bitmask'])]
cases['same_guesses_slower']=(t.loc[ix,'Solver5_bitmask']/t.loc[ix,'Solver3_bitmask']).idxmax()
fixtures={'known_fixture':'530070000600195000098000060800060003400803001700020006060000280000419005000080079',**cases}
for col,sub in d.groupby('collection'):
    ix=sub['Puzzle String'].drop_duplicates()
    small=[p for p in ix if p in gc.index and gc.at[p,'Naive']<2500]
    if small and small[0] not in fixtures.values():fixtures['collection_'+str(len(fixtures))]=small[0]
manifest=OUT/'sanity_manifest.txt'
manifest.write_text(''.join(k+'\t'+v+'\n' for k,v in fixtures.items()),encoding='utf-8')

compiler=Path(r'C:\Program Files\CodeBlocks\MinGW\bin\g++.exe')
exe=OUT/'sanity_check.exe'
sources=[ROOT/'tools/sudoku_benchmark_sanity.cpp']+[ROOT/f'src/{stem}.cpp' for stem in ['solver3','solver4','solver5','arraySolver3','arraySolver4','arraySolver5']]+sorted((ROOT/'src/new stuff').glob('*.cpp'))
env=os.environ.copy();env['PATH']=str(compiler.parent)+os.pathsep+env.get('PATH','')
if '--reuse-sanity' not in sys.argv:
    print('Building and running the current-code sanity check...',flush=True)
    build=subprocess.run([str(compiler),'-std=c++17','-O2','-static',*[str(p) for p in sources],'-o',str(exe)],capture_output=True,text=True,env=env,timeout=180)
    (OUT/'sanity_build_log.txt').write_text(build.stdout+build.stderr,encoding='utf-8')
    assert build.returncode==0,build.stderr
    run=subprocess.run([str(exe),str(manifest)],capture_output=True,text=True,env=env,timeout=120)
    (OUT/'sanity_results.tsv').write_text(run.stdout,encoding='utf-8')
    assert run.returncode==0,run.stderr
sanity=pd.read_csv(OUT/'sanity_results.tsv',sep='\t')
assert (sanity[['solved','valid','same_solution','trace_parity']]==1).all().all()
case_events={k:sanity[sanity['case']==k].set_index('solver') for k in cases}
assert case_events['naked_pair_help'].at['Solver4_bitmask','naked_pair_events']>0
assert case_events['xwing_help'].at['Solver5_bitmask','xwing_events']>0
fresh_agreement=[]
for label,p in fixtures.items():
    if label=='known_fixture':continue
    f=sanity[sanity['case']==label].set_index('solver')
    fresh_agreement.extend(f.at[s,'nodes']==g.at[p,s] for s in SOLVERS)
assert all(fresh_agreement)

plt.rcParams.update({'font.family':'DejaVu Sans','font.size':10,'axes.spines.top':False,'axes.spines.right':False,'axes.titleweight':'bold','figure.titlesize':15})
fig,axes=plt.subplots(1,2,figsize=(12,5),sharey=True)
for ax,stat in zip(axes,['mean','median']):
    vals=t.agg(stat).reindex(SOLVERS)
    ax.barh(range(8),vals,color=COLORS)
    ax.set_xscale('log');ax.set_yticks(range(8),[SHORT[s] for s in SOLVERS]);ax.invert_yaxis();ax.set_title(stat.capitalize()+' recorded time');ax.set_xlabel('Milliseconds (log scale)');ax.grid(axis='x',alpha=.18)
    for i,v in enumerate(vals):ax.text(v*1.07,i,fmt(v,4),va='center',fontsize=8)
    ax.set_xlim(vals.min()*.5,vals.max()*4)
fig.suptitle(f'Runtime across {len(t):,} distinct matched puzzles');fig.tight_layout();savefig('01_runtime')

clues=d.drop_duplicates('Puzzle String').set_index('Puzzle String').clues
clue_rows=[]
for clue,ix in clues.groupby(clues):
    ix=ix.index.intersection(t.index);tt=t.loc[ix]
    clue_rows.append([int(clue),len(ix),tt.Naive.median(),tt.Bitmask.median(),(tt.Naive/tt.Bitmask).median(),(tt.Bitmask/tt.Solver3_bitmask).median()])
clue_df=pd.DataFrame(clue_rows,columns=['clues','n','S1','S2','S1_over_S2','S2_over_S3'])
fig,axes=plt.subplots(1,2,figsize=(12,4.3))
for ax,c,title in zip(axes,['S1_over_S2','S2_over_S3'],['S1 / S2: bitmask checking','S2 / S3: propagation with MRV']):
    z=clue_df[clue_df.n>=10];ax.plot(z.clues,z[c],marker='o',color='#16836a');ax.axhline(1,color='#52606d',ls='--',lw=1);ax.set_xlabel('Initial clue count');ax.set_ylabel('Median paired speedup');ax.set_title(title);ax.grid(alpha=.18)
    if c=='S2_over_S3':ax.set_yscale('log')
fig.suptitle('Observed clue-count groups with at least 10 puzzles');fig.tight_layout();savefig('02_clue_counts')

transitions=[]
fig,axes=plt.subplots(1,2,figsize=(12,4.7))
for ax,a,b in zip(axes,['Solver3_bitmask','Solver4_bitmask'],['Solver4_bitmask','Solver5_bitmask']):
    change=g[b]-g[a];ratio=t[b]/t[a]
    ax.scatter(change,ratio,s=12,alpha=.45,color='#16836a');ax.axvline(0,color='#52606d',lw=1);ax.axhline(1,color='#52606d',ls='--',lw=1);ax.set_xscale('symlog',linthresh=1);ax.set_yscale('log');ax.set_xlabel('Change in guesses (new - previous)');ax.set_ylabel('New runtime / previous runtime');ax.set_title(SHORT[a]+' to '+SHORT[b]);ax.grid(alpha=.15)
    transitions.append({'a':a,'b':b,'n':len(change),'less':int((change<0).sum()),'same':int((change==0).sum()),'more':int((change>0).sum()),'faster':int((ratio<1).sum()),'less_faster':int(((change<0)&(ratio<1)).sum()),'less_slower':int(((change<0)&(ratio>1)).sum()),'same_slower':int(((change==0)&(ratio>1)).sum()),'median_time_ratio':float(ratio.median())})
fig.suptitle(f'Search savings and runtime changes on {len(t):,} complete heuristic traces');fig.tight_layout();savefig('03_tradeoffs')

correlations={}
fig,axes=plt.subplots(3,2,figsize=(11,11))
for ax,s in zip(axes.flat,MAIN):
    r=stats.spearmanr(gc[s],tc[s]);correlations[s]={'rho':float(r.statistic),'n':len(gc)}
    ax.scatter(gc[s]+1,tc[s],s=8,alpha=.3,color=COLORS[SOLVERS.index(s)]);ax.set_xscale('log');ax.set_yscale('log');ax.set_title(f'{SHORT[s]}: Spearman rho = {r.statistic:.3f}');ax.set_xlabel('Guesses + 1 (log scale)');ax.set_ylabel('Milliseconds (log scale)');ax.grid(alpha=.15)
axes.flat[-1].axis('off');fig.suptitle('More search generally costs time, but guesses do not measure all work');fig.tight_layout();savefig('04_guesses_runtime')

pairs=[('Naive','Bitmask'),('Bitmask','Solver3_bitmask'),('Solver3_bitmask','Solver4_bitmask'),('Solver4_bitmask','Solver5_bitmask')]+[(f'Solver{i}_array',f'Solver{i}_bitmask') for i in [3,4,5]]
rng=np.random.default_rng(20261009)
pair_stats=[]
for a,b in pairs:
    ratio=(t[a]/t[b]).to_numpy();boot=np.median(ratio[rng.integers(0,len(ratio),(2000,len(ratio)))],axis=1)
    logdiff=np.log(t[a])-np.log(t[b]);pval=stats.wilcoxon(logdiff).pvalue
    pair_stats.append({'a':a,'b':b,'n':len(ratio),'median_ratio':float(np.median(ratio)),'ci_low':float(np.quantile(boot,.025)),'ci_high':float(np.quantile(boot,.975)),'wins':int((ratio>1).sum()),'p':float(pval)})
ordered=sorted(range(len(pair_stats)),key=lambda i:pair_stats[i]['p']);running=0
for rank,i in enumerate(ordered):
    running=max(running,min(1,pair_stats[i]['p']*(len(ordered)-rank)));pair_stats[i]['p_holm']=running

quality=[]
for s in SOLVERS:
    z=d[d.Solver==s];quality.append([SHORT[s],len(z),int(z['Time (ms)'].notna().sum()),int(z.Events.isna().sum()),int((z.Samples==1).sum()),int(z.Samples.median())])
full_samples=pivot(d,'Samples');repeated=full_samples.index[(full_samples>=2).all(axis=1)]
sensitivity=[]
for label,ix in [('All matched timing puzzles',t.index),('Exclude any incomplete trace',common),('Require at least two samples for all solvers',repeated)]:
    z=t.loc[ix];sensitivity.append([label,len(z),z.Naive.mean(),z.Bitmask.mean(),(z.Naive/z.Bitmask).median(),(z.Bitmask/z.Solver3_bitmask).median()])
overlap=len(set(t.index)&set(nt.index))
provenance={str(p):{'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size} for p in [PRIMARY,NAMED,HISTORY,NOTEBOOK]}
summary={'title':'The Effects of Heuristics on Search Space and Work Done in Sudoku','primary':{'rows':len(d),'entries':d.Puzzle.nunique(),'distinct':len(t),'common_traces':len(common),'collections':d.collection.nunique()},'named':{'rows':len(named),'distinct':len(nt)},'overlap_between_cohorts':overlap,'quality':quality,'paired_comparisons':pair_stats,'transitions':transitions,'correlations':correlations,'sensitivity':sensitivity,'sanity':{'fixtures':len(fixtures),'solver_runs':len(sanity),'stored_valid':stored_valid,'node_matches':node_match,'fresh_historical_node_matches':len(fresh_agreement)},'cases':{k:{'puzzle':p,'id':names[p]} for k,p in cases.items()},'source_files':provenance}
(OUT/'analysis_summary.json').write_text(json.dumps(summary,indent=2),encoding='utf-8')
print(json.dumps(summary,indent=2))

# The written report is assembled from these calculated values below.
report=[]
def add(s=''):report.append(s+'\n')
add('# The Effects of Heuristics on Search Space and Work Done in Sudoku\n')
add('**Benchmark analysis | 9 October 2026**\n')
add('## Clear answer\n')
p23=pair_stats[1];tr34,tr45=transitions
add(f'Candidate propagation with MRV substantially reduces the explored search and recorded runtime on this benchmark. Across {len(t):,} distinct matched collection puzzles, Solver 3 with bitmask candidates has a median paired speedup of **{p23["median_ratio"]:.2f}x over basic bitmask backtracking**. Adding naked pairs or X-Wing reduces guesses on many puzzles, but usually increases runtime: Solver 4 is faster than Solver 3 on {int((t.Solver4_bitmask<t.Solver3_bitmask).sum()):,}/{len(t):,} puzzles, and Solver 5 is faster than Solver 4 on {int((t.Solver5_bitmask<t.Solver4_bitmask).sum()):,}/{len(t):,}. The useful conclusion is that additional deduction must earn back its processing cost on the actual puzzle distribution.\n')
add('These measurements describe explored branches and recorded work. They do not measure the complete mathematical solution space, CPU instructions, or energy use.\n')
add('## 1. Research frame\n')
add('**Title:** The Effects of Heuristics on Search Space and Work Done in Sudoku.\n')
add('1. **RQ1:** How does bitmask representation affect runtime when search policy is held fixed, and how does that effect vary with initial clue count?\n2. **RQ2:** How much do propagation and MRV change explored search and runtime relative to basic backtracking?\n3. **RQ3:** How do naked pairs and X-Wing change guesses and runtime on the same puzzles?\n4. **RQ4:** When do search savings translate into runtime savings, and when does added deduction work outweigh them?\n')
add('**Working hypothesis:** Propagation with MRV will reduce search substantially; representation changes will have a smaller and workload-dependent effect; richer deduction sets will help selected puzzles but may increase total runtime when their search savings are insufficient. This is a working interpretation developed while examining existing data, not a preregistered hypothesis tested on a new holdout sample.\n')
add('## 2. Data, correctness, and metric definitions\n')
add(table(['Experiment','Rows','Distinct puzzles','Role'],[['Collection benchmark',f'{len(d):,}',f'{len(t):,}','Main analysis'],['Supplied named-puzzle benchmark',len(named),len(nt),'Separate illustrative experiment']]))
add(f'The collection file contains {d.Puzzle.nunique():,} sampled collection entries from {d.collection.nunique()} collections. There are {d.Puzzle.nunique()-len(t)} repeated memberships beyond the distinct-puzzle count. The named-puzzle attachment and collection benchmark share {overlap} exact puzzle strings. They are not pooled, and timings from the two experiments must not be subtracted as if they came from the same build or measurement session.\n')
add('**Pairing and aggregation:** A row is one solver on one collection entry. Repeated memberships for the same exact puzzle and solver are collapsed to the median of their recorded timings. Overall results use the same distinct puzzles for all eight solvers. Search counters are likewise collapsed by median; their repeated values were checked for consistency. Collection summaries retain membership and should not be summed to reconstruct a distinct-puzzle total.\n')
add('**Timing:** The master runner normally exports the middle ordered trace-free timing from repeated solves, or a preserved calibration measurement if a later step fails. It measures around the solver call and collects trace metrics separately. The file does not contain the raw repeats, so the original mean runtime within each puzzle/run cannot be reconstructed. The mean below is a mean across per-puzzle recorded timings; it is not a mean of all raw timed repetitions.\n')
add('**Sampling:** The matching historical metrics file records seed **20260928**, requested **200 entries per collection**, a **15,000 ms** worker timeout, up to **31 timed runs**, and a **250 ms** timing target. The master source uses reservoir sampling with the seed combined with a filename hash. These are recovered settings from the matching historical records, not proof of a preserved invocation for the newer timing file. The sample was drawn by collection, not equally by clue count. Clue counts are derived from the 81-character strings and used for descriptive stratification.\n')
add('### Fresh sanity check\n')
add(f'A fresh GCC 14.2.0 C++17 build, using -O2 and static linking, checked **{len(fixtures)} fixtures across eight solvers ({len(sanity)} solver/fixture pairs)**. Every pair returned a valid completed grid, preserved givens, agreed on the solution, and produced matching traced/untraced results and node counts. The known fixture also matched its independently supplied expected grid. Each pair was executed once without tracing and once with tracing. The other {len(fixtures)-1} fixtures were selected from the benchmark, including the four case studies and inexpensive collection examples. All {len(fresh_agreement)} benchmark fixture/solver node counts matched their stored values. This is a current-code sanity check, not a fresh rerun of every historical timing or an exhaustive correctness proof.\n')
add(f'An independent audit also validated all **{stored_valid:,} stored final boards** in the older metrics file, including row/column/box constraints and original givens. All eight stored solutions agreed per entry. The older and newer files match on all {len(d):,} puzzle/solver keys and exact puzzle strings, and all {node_match:,} available newer guess counts equal the older node counts. This supports using the stored counters as explored-branch counts. It does not prove historical executable/source identity or retroactively certify the timing session.\n')
add('### What each counter means\n')
add(table(['Metric','Meaning and proper use'],[['Search nodes / Guesses','A legal guessed placement entering a recursive branch. In current code, all eight solvers use this definition; rejected digit tests and deterministic deductions are excluded.'],['Dead ends','Guess events marked as leading to conflict; failed guesses, not every trace event.'],['Max depth','Maximum recorded event depth. Baseline attempts start at depth 0 while heuristic guesses start at depth 1; compare within a family and avoid interpreting a one-level difference as an algorithmic benefit.'],['Placements','Events with a placement payload. Includes attempts and backtracks in the current implementation; not unique filled cells.'],['Eliminations','Explicit candidate removals attached to technique events; not all internal candidate updates.'],['Events','Trace records. One record may contain several changes; records differ between solver families. Not CPU operations.'],['Zero-guess solve','Guesses equals zero on a successful solve; success must be checked independently because the newer timing CSV has no solved field.']]))
add('The older project prose says that baseline nodes count all attempted digits. That is inconsistent with the current source and with the matching collection counters. Do not copy that statement into the new report. For the supplied 24-puzzle table, `Guesses` is treated as the labeled branch counter; its `Placements`/`Events` clearly follow a different logging convention from current baseline traces. Absolute event counts are therefore not pooled across experiments or presented as equal-cost units.\n')
add('### Missingness and repetition\n')
add(table(['Solver','Rows','Positive timing rows','Incomplete traces','Single-sample timings','Median samples'],quality))
add(f'All {len(d):,} collection timing rows are positive. Seven traces are incomplete, belonging to four puzzles. Overall timing comparisons retain their recorded timings; common-trace comparisons use **{len(common):,}** puzzles. The timing CSV lacks explicit solved/valid fields. Missing trace statistics are not replaced with zero, and a positive timing alone is not proof of success.\n')
add('## 3. RQ1: Representation and runtime\n')
add('### Main timing comparison\n')
timing_rows=[]
for s in SOLVERS:
    v=t[s];timing_rows.append([SHORT[s],fmt(v.mean(),6),fmt(v.median(),6),fmt(v.quantile(.95),6),fmt((t.Naive/v).median(),2)])
add(table(['Configuration','Mean ms','Median ms','P95 ms','Median paired S1/time'],timing_rows))
add('![Mean and median runtime](figures/01_runtime.png)\n')
add('Mean, median, and P95 answer different questions. The mean is sensitive to expensive tail puzzles; the median describes the middle puzzle; P95 marks the slowest 5% boundary. Paired speedup is the median of puzzle-by-puzzle ratios, not a ratio of aggregate medians.\n')
p12=pair_stats[0]
add(f'S1 and S2 have identical guess counts on all **{len(gc):,} common complete traces**. S2 is faster on **{p12["wins"]:,}/{len(t):,}** timing pairs. Its median paired speedup is **{p12["median_ratio"]:.3f}x**, with a paired puzzle-bootstrap interval of {p12["ci_low"]:.3f}-{p12["ci_high"]:.3f}x. That is a small typical representation benefit on this corpus. A speedup ratio of 1.026x means about 2.6% more throughput, or about 2.5% less elapsed time; these percentages are not interchangeable.\n')
add('The paired heuristic implementations produce identical guesses on all matched puzzles. Their timing comparison concerns the implemented candidate representation, initialization, updates, and state copying together. It does not isolate one bitwise instruction.\n')
add(table(['Stage','Median array/bitmask time','Bitmask faster / puzzles'],[[f'Solver {i}',fmt(pair_stats[i+1]["median_ratio"],3),f'{pair_stats[i+1]["wins"]:,}/{len(t):,}'] for i in [3,4,5]]))
add('### Average solving times by source collection\n')
collection_rows=[]
for collection,sub in d.groupby('collection'):
    group_times=pivot(sub,'Time (ms)').dropna()
    collection_rows.append([collection.replace('.txt',''),len(group_times),*[fmt(group_times[s].mean(),4) for s in MAIN]])
add(table(['Collection','Distinct entries','S1 mean ms','S2 mean ms','S3 mean ms','S4 mean ms','S5 mean ms'],collection_rows))
add('Each row uses matched puzzles within that collection; stages 3-5 use bitmask candidates. Collections overlap, so their row counts need not sum to the overall distinct count. These source labels are not calibrated difficulty levels.\n')
add('### Per-clue-count comparison\n')
add('![Clue-count comparisons](figures/02_clue_counts.png)\n')
add(table(['Clues','Distinct puzzles','S1 median ms','S2 median ms','Median S1/S2','Median S2/S3 bitmask'],[[r[0],r[1],fmt(r[2],5),fmt(r[3],5),fmt(r[4],3),fmt(r[5],2)] for r in clue_rows]))
add('The plot displays groups with at least ten puzzles; the table retains all observed groups. These are post-hoc groups from a collection sample, not balanced random samples at each clue count. Clue count does not hold puzzle structure or collection composition fixed, so a trend cannot establish that clue count causes runtime or difficulty.\n')
add('## 4. RQ2: Propagation with MRV and explored search\n')
node_rows=[]
for s in MAIN:
    v=gc[s];de=pivot(d,'Dead ends').loc[common,s]
    node_rows.append([SHORT[s],fmt(v.mean(),3),fmt(v.median(),1),fmt(de.mean(),3),int((v==0).sum())])
add(table(['Configuration','Mean guesses','Median guesses','Mean failed guesses','Zero-guess cases'],node_rows))
reduction=100*(1-gc.Solver3_bitmask/gc.Bitmask)
add(f'On the {len(common):,} common complete traces, Solver 3 reduces guesses relative to S2 by a **median {reduction.median():.4f}%** across paired puzzles. On all {len(t):,} heuristic trace records, the numbers of zero-guess cases are **{int((g.Solver3_bitmask==0).sum())} for S3**, **{int((g.Solver4_bitmask==0).sum())} for S4**, and **{int((g.Solver5_bitmask==0).sum())} for S5**. These totals use the full heuristic trace set; the table above uses the smaller common subset.\n')
add(f'Solver3_bitmask has a median paired speedup of **{p23["median_ratio"]:.2f}x over S2**. This is the combined effect of propagation, candidate-restricted branching, and MRV. The current benchmark does not isolate MRV, even though a first-empty-cell control exists in source. The measured node count describes the explored tree, not the number of all possible assignments.\n')
add('## 5. RQ3: Incremental deductions\n')
add(table(['Transition','Fewer guesses','Same guesses','More guesses','Faster runtime','Median new/old time'],[[SHORT[z['a']]+' to '+SHORT[z['b']],z['less'],z['same'],z['more'],f'{z["faster"]}/{z["n"]}',fmt(z['median_time_ratio'],3)] for z in transitions]))
add('![Search and runtime trade-offs](figures/03_tradeoffs.png)\n')
for z in transitions:
    add(f'For {SHORT[z["a"]]} to {SHORT[z["b"]]}, **{z["less_faster"]}** puzzles have fewer guesses and lower runtime, while **{z["less_slower"]}** have fewer guesses but higher runtime. Another **{z["same_slower"]}** have the same guess count and higher runtime. These comparisons use all {len(t):,} puzzles because the heuristic traces are complete.\n')
add('Additional deductions can change later MRV choices and the branch path. Soundness does not guarantee monotonic guess counts, and a guess-count increase is not itself an implementation defect. Equal guesses also do not prove an identical search tree. A slower solve with equal guesses is an observed cost without a guess-count benefit; calling it pure detector overhead would require more detailed instrumentation.\n')
add('## 6. RQ4: Search effort versus time\n')
add('### Recorded activity beyond guesses\n')
work_rows=[]
for s in SOLVERS:
    work_rows.append([SHORT[s],*[fmt(pivot(d,m).loc[common,s].mean(),2) for m in ['Max depth','Placements','Eliminations','Events']]])
add(table(['Configuration','Mean recorded max depth','Mean placement payloads','Mean explicit eliminations','Mean events'],work_rows))
add('These averages use the same 1,909 common complete traces. Depth origins and placement/event conventions differ between baselines and heuristic stages, as defined above. Compare these measures within a solver family; do not sum them into a total operation count or equate one baseline event with one heuristic event. The array/bitmask pairs have matching trace activity, helping separate search behavior from representation cost.\n')
add('![Guesses against runtime](figures/04_guesses_runtime.png)\n')
add(table(['Solver','Spearman rank correlation: guesses vs time','Puzzles'],[[SHORT[s],fmt(correlations[s]['rho'],5),len(common)] for s in MAIN]))
add('The correlations are calculated within each solver, using the same complete-trace puzzles. The plot uses guesses + 1 only to display zero-guess cases on a log axis; the correlation uses unshifted guesses. Positive correlation means harder search generally accompanies longer runtime. It does not estimate the time saved by removing a guess, and it does not establish that all work is proportional to guesses. Deduction scans, candidate updates, and state copying remain relevant.\n')
add('There is no defensible universal guess-count threshold where heuristics stop being worth it. The paired transition plot gives the answer puzzle by puzzle: a deduction is beneficial for runtime when its observed runtime ratio is below one. Explaining the mechanism requires the associated trace and, for exact costs, instrumentation of technique calls and time spent in each technique.\n')
add('## 7. Four case-study puzzles\n')
add('Cases below were selected from the collection benchmark to illustrate distinct behaviors, with a cap on baseline search cost so they could be freshly checked. The helpful cases require fewer guesses and lower stored runtime; the last case requires equal guesses and higher stored runtime. These are deliberately selected examples, not estimates of how common each behavior is. Fresh traces confirm that the named pair/X-Wing rules actually fire.\n')
for label,p in cases.items():
    add('### '+label.replace('_',' ').capitalize()+'\n')
    add(f'**Puzzle:** {names[p]}  \n**Initial clues:** {clues[p]}  \n**81-cell input:** `{p}`\n')
    e=case_events[label]
    add(table(['Stage','Guesses','Dead ends','Stored time ms','Fresh naked-pair events','Fresh X-Wing events'],[[SHORT[s],int(g.at[p,s]),int(pivot(d,'Dead ends').at[p,s]),fmt(t.at[p,s],6),int(e.at[s,'naked_pair_events']),int(e.at[s,'xwing_events'])] for s in ['Solver3_bitmask','Solver4_bitmask','Solver5_bitmask']]))
    if label=='zero_guess':add('Singles and pointing deductions finish this puzzle without branching in all three stages. Additional techniques have no guess-count saving to achieve here.\n')
    elif label=='naked_pair_help':add('Adding naked pairs produces fewer guesses and lower measured runtime. The fresh Solver 4 trace contains naked-pair events, supporting the technique-level explanation. Other later choices can also change, so this is the effect of enabling the stage as a whole.\n')
    elif label=='xwing_help':add('Adding X-Wing to Solver 4 produces fewer guesses and lower measured runtime. The fresh Solver 5 trace contains X-Wing events. This confirms actual use of the rule rather than inferring it only from a smaller guess count. Solver 5 is still slower than Solver 3 on this puzzle, illustrating that improving one stage does not establish the best configuration overall.\n')
    else:add('Solver 5 takes longer than Solver 3 with no reduction in guesses. This is a cost case for richer deduction on this input. It does not prove the traces are identical or that every additional microsecond comes from detector scanning.\n')
add('## 8. What the supplied 24-puzzle benchmark says\n')
add('This attachment is a separate historical experiment containing 24 named puzzles and 192 rows. Its timings use up to 100 samples, unlike the larger collection file. The older report describes these as selected behavioral examples; they are not a random sample of Sudoku puzzles.\n')
add(table(['Configuration','Mean ms','Median ms','Mean guesses'],[[SHORT[s],fmt(nt[s].mean(),4),fmt(nt[s].median(),4),fmt(ng[s].mean(),3)] for s in SOLVERS]))
named_ratio=nt.Naive/nt.Bitmask
ns=pivot(named,'Samples');nrepeat=ns.index[(ns[['Naive','Bitmask']]>=2).all(axis=1)]
anom=named[(named.Puzzle.str.contains('weak link',case=False)) & (named.Solver.isin(['Naive','Bitmask']))]
add(f'S2 is faster on **{int((named_ratio>1).sum())}/24** named puzzles, with a median paired speedup of **{named_ratio.median():.3f}x**. However, the mean S2 time is **{nt.Bitmask.mean():.3f} ms**, above S1 at **{nt.Naive.mean():.3f} ms**. The single-sample `GX- Cycle (weak link) x7` timings are **{nt.loc[anom["Puzzle String"].iloc[0],"Naive"]:.4f} ms for S1** and **{nt.loc[anom["Puzzle String"].iloc[0],"Bitmask"]:.4f} ms for S2**, reversing the pooled mean. This observation is retained. The attachment does not contain the remeasurement that would justify replacing it.\n')
add(f'If the baseline comparison is restricted to puzzles with at least two timing samples for both baselines, **{len(nrepeat)} puzzles** remain and the median S1/S2 ratio is **{(nt.loc[nrepeat,"Naive"]/nt.loc[nrepeat,"Bitmask"]).median():.3f}x**. This is a sensitivity result, not a correction to the raw table.\n')
add(table(['Transition','Fewer guesses','Same guesses','More guesses','Faster runtime / 24'],[[f'S{i} to S{i+1}',int((ng[f'Solver{i+1}_bitmask']<ng[f'Solver{i}_bitmask']).sum()),int((ng[f'Solver{i+1}_bitmask']==ng[f'Solver{i}_bitmask']).sum()),int((ng[f'Solver{i+1}_bitmask']>ng[f'Solver{i}_bitmask']).sum()),int((nt[f'Solver{i+1}_bitmask']<nt[f'Solver{i}_bitmask']).sum())] for i in [3,4]]))
add('The named set shows useful deductions without guaranteed runtime improvement, just as the collection set does. Its much larger typical array/bitmask timing differences do not transfer automatically to the collection experiment. They may reflect puzzle selection, build changes, machine state, or measurement differences. Those causes are not separately identifiable from the supplied summaries.\n')
add('## 9. Robustness and statistical support\n')
add(table(['Collection sensitivity subset','Puzzles','S1 mean ms','S2 mean ms','Median S1/S2','Median S2/S3'],[[r[0],r[1],fmt(r[2],4),fmt(r[3],4),fmt(r[4],3),fmt(r[5],2)] for r in sensitivity]))
add('The repeated-sample subset removes any puzzle with fewer than two samples for any solver. This is stronger than excluding incomplete traces. Both filters preferentially remove expensive baseline cases, so their estimates describe easier retained subsets and are not unbiased replacements for the full corpus.\n')
add(table(['Paired comparison A/B','Median A/B','95% puzzle-bootstrap interval','Holm-adjusted Wilcoxon p'],[[SHORT[z['a']]+' / '+SHORT[z['b']],fmt(z['median_ratio'],3),f'{z["ci_low"]:.3f}-{z["ci_high"]:.3f}',('<1e-300' if z['p_holm']<1e-300 else f'{z["p_holm"]:.3g}')] for z in pair_stats]))
add('Ratios above one favor B. Intervals use 2,000 paired puzzle bootstrap resamples with seed 20261009. Tests use paired log timings with Holm correction across the seven listed comparisons. Exact repeated puzzle strings are collapsed first. Related puzzle families may still weaken independence; signed-rank interpretation also depends on the paired log-difference distribution. These exploratory statistics describe this sample and do not repair historical timing noise. The intervals represent variation across sampled puzzles, not repeatability on new hardware or uncertainty from raw timing repetitions. The effect size and observed exceptions carry the practical conclusion.\n')
add('## 10. Discussion and limitations\n')
add('The most substantial improvement is associated with propagation plus MRV, which reduces explored search enough to offset its extra work on these inputs. Adding naked pairs and X-Wing continues to reduce typical guess counts and increases the number of deduction-only cases. However, the extra work often outweighs those search savings in elapsed time. Representation effects are smaller and vary across experiments. These findings support evaluating deduction sets against the expected input distribution rather than treating every stronger technique as an automatic speed improvement.\n')
add('- **Existing measurements:** The historical performance experiments were analysed, not rerun in full. Fresh executions establish a small correctness and counter-parity check.\n- **Success schema:** The newer CSV has no explicit solved/valid fields. Matching older stored boards are useful supporting evidence but cannot prove the newer session had identical outcomes.\n- **Sampling:** Ten collections and a separately selected named set are represented. This is not a balanced clue-count design or a random sample from all Sudoku puzzles.\n- **Timing:** Different repetition counts, calibration-only measurements, fixed solver order, missing raw timing repeats, and incomplete historical environment records limit precise attribution.\n- **Deduction attribution:** S2 to S3 combines propagation with MRV. S4/S5 isolate enabling additional configured rules, not the CPU time of each detector.\n- **Work metrics:** Event payloads and depth origins differ by solver family or logging version. Branch counts are useful, but event totals and time/event are not equal-cost operation counts.\n- **Selection and dependence:** Exact duplicates are removed for pooled comparisons. Related puzzle families can remain dependent, and case studies were selected after examining results.\n')
add('## 11. What to collect in the next benchmark\n')
add('Retain the current collection comparison as a pilot. Freeze the exact input manifest, seed, compiler/build identity, and machine details for the next run. If the question specifically concerns clue count, sample a defined number of puzzles within each clue stratum; report unavailable or small strata rather than calling the current collection sample balanced.\n')
add('Record one row per puzzle/configuration with explicit solved, valid, and outcome status; the branch-node count; failed guesses; normalized guess depth; zero-guess status; and actual timing sample count. Preserve all raw trace-free timings so both mean and median within a puzzle can be reported. Include the Solver 3 first-empty/MRV control if making a separate MRV claim. Counterbalance solver order and retain timeout records.\n')
add('For a direct work-done explanation, additionally record technique call counts, successful deductions, individual candidate removals, and time spent in each technique with instrumentation overhead assessed separately. This will allow detector overhead to be measured rather than inferred from equal guesses.\n')
add('## 12. Conclusion\n')
add(f'The collection benchmark supports a clear result: **propagation with MRV sharply reduces explored search and gives a typical {p23["median_ratio"]:.2f}x runtime improvement over basic bitmask backtracking, while richer deduction sets often trade fewer guesses for longer runtime.** Naked pairs and X-Wing have useful cases, but their value depends on the puzzle and the work needed to detect them. The appropriate claim concerns this implemented solver family and these benchmark inputs; it is not a universal ranking of Sudoku algorithms.\n')
add('## Evidence and reproducibility\n')
add('- Main measurements: [collection benchmark](../../results/master_collection_benchmark.csv).\n- Stored validity and matching branch counters: [historical metrics](../../results/master_collection_metrics.csv).\n- Separate experiment: the supplied 192-row named-puzzle attachment.\n- Supporting calculations and hashes: [analysis summary](analysis_summary.json).\n- Fresh executions: [sanity results](sanity_results.tsv) and [frozen fixture manifest](sanity_manifest.txt).\n- Reproduction: run `tools/analyze_sudoku_benchmarks.py` with the bundled Python environment and the dependencies described in its imports. It rebuilds the sanity harness, checks the stored data, and regenerates this report and figures.\n')
(OUT/'sudoku_heuristics_analysis.md').write_text('\n'.join(report).replace('\n\n\n','\n\n'),encoding='utf-8')
print('REPORT',OUT/'sudoku_heuristics_analysis.md')
