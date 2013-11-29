<h1>MPI WC++</h1>
MPI implementation of Map-reduce program.

<ol>
<li>Read a file on n nodes using MPI_IO</li>
<li>Use offsets in read to make sure every whole word is read</li>
<li>Count words on input nodes</li>
<li>Send result map to routing nodes</li>
<li>Routing nodes sort input values and send to reduce nodes</li>
<li>Reduce nodes gather data, sum the values and write to output file</li>
</ol>


LD_LIBRARY_PATH="./protobuf-2.5.0/src/.libs/:$LD_LIBRARY_PATH"

<em>mapparar lesa og telja, skipta svo upp í fötur og senda á reddara. Þegar þeir eru búnir senda þeir tóman lista. Reddarar lesa á fullu þar til þeir hafa fengið n tóma lista (þar sem n er fjöldi mappara). Þá senda þeir allt sem þeir hafa á eina nóðu sem gerir ekkert annað en að bíða eftir n skilaboðum (þar sem n er fjöldi reddara). Svo þegar allt er komið er skrifað í skrá. Nota google protocol buffers fyrir binary formatið, túlk í python.</em>