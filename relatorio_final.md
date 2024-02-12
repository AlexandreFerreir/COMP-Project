# Relatório Final

## I. Gramática Re-escrita 

- A gramática começa com uma produção inicial “Program” que dá ‘shift’ para “FunctionsAndDeclarations”; 

- A produção “FunctionsAndDeclarations” contém as produções “FunctionDefinition”, “FunctionDeclaration” e “Declaration”; 

- A produção “FunctionDefinition” e “FunctionDeclaration” sofreram alteração devido à eliminação da produção "FunctionDeclarator" (*1), logo, contêm o seu corpo. Já a“FunctionBody” permaneceu inalterada, implicando que “DeclarationAndStatements” seja opcional; 

- A produção “DeclarationAndStatements” utiliza recursão à direita;

- A Produção “ParameterList” utiliza uma produção auxiliar "ParameterListAux" de forma a criar uma lista de produções“ParameterDeclaration”. Esta última produção não sofreu nenhuma alteração, implicando que “IDENTIFIER” seja opcional; 

- A produção “Declaration” faz uso de uma produção auxiliar “DeclarationAux”, permitindo assim criar uma lista de produções “Declarator” separadas por uma ‘Comma’ (vírgula), com recursão à esquerda. Foi adicionado também, um estado “error”, que é executado caso haja alguma erro sintático nesta produção; 

- As produções “TypeSpec” e “Declarator” não sofreram nenhuma alteração;
 
- Na produção “Statement” foi adicionado um estado “error” que se comporta de igual forma ao anterior. Para esta produção foi necessário criar mais três produções auxiliares. "StatementComErro", que adiciona um estado de erro, implicando que seja esta a produção a ser utiliza em várias outras produções, em vez de "Statement", uma vez que resolve um conflito ‘shift/reduce’ existente. "StatementList" e "StatementAux" são constituídos por corpos iguais que geram uma lista de "StatementComErro", no entanto, o "StatementAux" diferencia-se por ser uma produção que evita a criação de nós StatList supérfluos, ou seja, com menos de dois statements.

- Na produção "expression" foram adicionados dois estados de “error” que se comportam da mesma forma que os anteriores. Foi necessário criar duas produções auxiliares com corpos semelhantes mas objetivos distintos: “expressionAux” que  permite a criação de uma lista de "expression" separadas por uma ‘Comma’ (vírgula) e “expressionCall” que permite que quando haja uma expressão do tipo ‘Call’ seja gerada uma lista de "expression" separadas por uma ‘Comma’ (vírgula), no entanto cada expressão é filha do nó ‘Comma’. Nesta última produção os nós serão todos ‘irmãos’ uns dos outros. 

(*1) A regra "FunctionDeclarator" foi eliminada para evitar uma maior complexidade na criação de nós, logo, todas as outras regras que a utilizam, passaram a utilizar o seu respetivo corpo.



## II. Algoritmos e Estruturas de Dados da AST e Tabela de Símbolos

**AST**
(a estrutura utiliza um novo atributo no nó, a "annotation" do tipo "category")

- Caso seja preciso adicionar nós irmãos, são criados nós "Unknown" temporários que ficam como pais dos irmãos, e mais "acima" a funcão "assignBrothers" seleciona todos os nós que deviam ser irmãos e associa-os ao mesmo pai;

- Na produção Statement para verificar a existência de nós Nulos utilizam-se duas funcões ("checkChildren" e "returnCheckChildren"), que distinguem o uso da produção "StatementComErro" ou "StatementAux", através da contagem de nós não nulos e retorno de Nulos;

- Perante a necessidade da presença do TypeSpec em todos os Declarators, no próprio Declarator é criado um nó "Unknown" correspondente ao TypeSpec que é atualizado mais "acima" através da função "assignTypeSpec" que percorre os nós filhos e atribui o TypeSpec desejado;

- Para limpeza da memória associada à AST utiliza-se a função "deallocate";

- Para imprimir a AST é chamada a função "print_tree_with_annotations" que imprime a categoria do nó, token e anotação se existente, precedidos por pontos que indicam a profundidade do nó.

**Tabela de Símbolos**
(existe uma lista ligada de tabelas em que cada tabela contém uma lista de símbolos)

- Cada tabela possui o nome da função associada e a sua lista de símbolos;

- Cada símbolo tem como atributos o seu identificador, se é parametro ou não, o seu tipo, e tipo de retorno e ponteiro para os parâmetros na AST caso seja uma declaração de uma função (especificamente na primeira tabela, que é tabela dos símbolos globais); 

- A função "check_program" percorre toda a AST de forma recursiva atribuindo anotações aos devidos nós e criando e atualizando as tabelas de símbolos, passando o ponteiro da tabela local na própria chamada recursiva;

- Através de um "switch" todos os tipos de nós com comportamentos específicos são controlados;

- Dado o comportamento recursivo da funcão "check_program", no caso dos nós "If" e "While" em que é necessário percorrer os filhos individualmente em caso de possíveis erros, foi necessário criar uma versão da check_program que atua a partir de um determinado nó e não apenas nos seus filhos ("check_program_on_node");

- A função principal "check_program" utiliza várias outras funções auxiliares, como por exemplo, as funções "check_if_existent_function_error" e "check_parameters_on_function_call" para controlo de erros nas funções, as funções "int_short_char" e "conversion_declaration_types" para controlo de conversões de tipos;

- Da mesma forma que na AST, existe uma função que liberta a memória associada às tabelas.



## III. Geração de Código

- Primeiramente é chamada a função "codegen_program" que declara as funções "getchar" e "putchar", e percorre a AST chamando as funções "codegen_functiondefinition" e "codegen_globaldeclaration" conforme os possíveis nós ("codegen_functiondeclaration" não implementada).

- Caso seja variável global a função "codegen_globaldeclaration" imprime a sua declaração, da mesma forma caso seja declaração local de variáveis e parâmetros (chamados na função "codegen_funcbody") são impressos pela função "codegen_localdeclaration", com o "alloca", "store" e possível "load" caso exista alguma expression na declaração;

- A função "codegen_statement" controla as expressions ("codegen_expression") e os nós If, While e Return com as funções "codegen_if", "codegen_while" e "codegen_return", respetivamente.

- A numeração de todas as variáveis temporárias são controladas pela variável global "temporary" que é incrementada a cada operação.

- De forma a facilitar e unificar o retorno nas expressions, mesmo que uma expressão seja um valor literal, é sempre criada uma variável temporária para este e retornado o inteiro identificador.

- A função "remove_plica" faz a tradução entre "CharLit" e "Int" chamada dentro da função "initialized_value" que retorna a string com a conversão correspondente ao valor dado.

- Através das funções "type_to_llvm" e "type_alignment", conforme o tipo da variável as funções retornam os respetivos tipos em LLVM IR ou o número de bytes necessários a alocar. 

- O comportamento em nós "Call" não está genericamente implementado, possui apenas ação caso a chamada seja à função "putchar", de forma a ser capaz de testar as restantes funcionalidades.

- Ao longo das chamadas das funções necessárias o ponteiro da tabela local é sempre transmitido de forma a ser possível consultar/atualizar o identificador temporário das variáveis presente como atributo do símbolo na tabela local. 

