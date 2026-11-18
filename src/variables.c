#include "shell.h"

/* ---------- Feature-8: Shell variables ----------
 * Variables ek singly linked list mein save hote hain: (naam, value, next)
 *     MSG=hello   -> set_var("MSG", "hello")
 *     echo $MSG   -> chalane se pehle "$MSG" ko get_var("MSG") se badal do
 */
typedef struct Var {
    char* name;
    char* value;
    struct Var* next;
} Var;

static Var* var_list = NULL;            // list ka head (pehla node)

void set_var(const char* name, const char* value) {
    for (Var* v = var_list; v != NULL; v = v->next) {
        if (strcmp(v->name, name) == 0) {       // pehle se hai -> value badlo
            free(v->value);
            v->value = strdup(value);
            return;
        }
    }
    Var* v = malloc(sizeof(Var));               // naya variable -> list ke shuru mein
    v->name  = strdup(name);
    v->value = strdup(value);
    v->next  = var_list;
    var_list = v;
}

char* get_var(const char* name) {
    for (Var* v = var_list; v != NULL; v = v->next)
        if (strcmp(v->name, name) == 0)
            return v->value;
    return getenv(name);                // na mile to environment se ($HOME, $USER...)
}

void print_vars(void) {
    for (Var* v = var_list; v != NULL; v = v->next)
        printf("%s=%s\n", v->name, v->value);
}

/* Kya ye token NAME=value jaisa assignment hai?
 * NAME letter ya '_' se shuru ho, aur sirf letters, digits, '_' ho. */
int is_assignment(const char* tok) {
    const char* eq = strchr(tok, '=');
    if (eq == NULL || eq == tok) return 0;              // '=' nahi, ya shuru mein '='
    if (!(isalpha((unsigned char)tok[0]) || tok[0] == '_')) return 0;
    for (const char* p = tok; p < eq; p++)
        if (!(isalnum((unsigned char)*p) || *p == '_')) return 0;
    return 1;
}

/* "NAME=value" ko tor kar save karo */
void do_assignment(const char* tok) {
    char* copy = strdup(tok);
    char* eq = strchr(copy, '=');
    *eq = '\0';                         // copy = "NAME", eq+1 = "value"
    set_var(copy, eq + 1);
    free(copy);
}

/* arg '$' se shuru ho to variable ki value ki NAYI malloc string do
 * (variable na ho to khali string ""). Warna NULL. */
char* expand_arg(const char* arg) {
    if (arg[0] != '$' || arg[1] == '\0')
        return NULL;
    char* value = get_var(arg + 1);
    return strdup(value ? value : "");
}
