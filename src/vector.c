extern void _stext();

#pragma section const {vector}

void (* const @vector vector_table[32])() = {
    _stext,			// RESET
};
