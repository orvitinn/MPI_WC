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
