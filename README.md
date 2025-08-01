# ticket_lock
Ticket lock for fair FIFO locking where each lock acquisition is assigned a unique ticket number from an atomic counter and requests are served in order of their ticket, with each unlock incrementing the serving counter so that the next ticket can proceed
