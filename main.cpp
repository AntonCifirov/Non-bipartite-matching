#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <algorithm>
using namespace std;
using namespace std::chrono;

struct Grafas 
{
    int n, m;   // n - kiek turime virsuniu duotame grafe, m - maximalus indexas po sutraukimo visu ciklu
    
    vector<int> mate;  // n ilgio masivas, jei virsune v neuždengta (uncovered, exposed) mate[v] = -1, jei uzdengta mate[v] rodo virsune sujungta su v poravime
    vector<vector<int>> vertices_in_blossom;
    vector<int> parent, label, blossom_containing_vertex;
    vector<vector<int>> edge_matrix;
    Grafas(int n) : n(n) 
    {
        m = n + n / 2;
        mate.assign(n, -1);  // užpildom mate vektoriu -1 ais
        vertices_in_blossom.resize(m);
        parent.resize(m);
        label.resize(m);
        blossom_containing_vertex.resize(m);
        edge_matrix.assign(m, vector<int>(m, -1));
    }
    void add_edge(int u, int v) 
    {  // kuriame nesuporuota briauna
        edge_matrix[u][v] = u;
        edge_matrix[v][u] = v;
    }
    void match(int u, int v) 
    {   // keiciame briauna ne poravime i briauna poravime
        edge_matrix[u][v] = edge_matrix[v][u] = -1;
        mate[u] = v;
        mate[v] = u;
    }


    //             Sekiame kelią iki šaknies
    vector<int> path_to_root(int x)   // Sudetingumas O(n)
    {
        vector<int> xpath_to_root;  // vx - kelias iki saknies nuo x visunes
        while (true) 
        {
            while (blossom_containing_vertex[x] != x) 
                x = blossom_containing_vertex[x];

            if (!xpath_to_root.empty() && xpath_to_root.back() == x)  
                break;

            xpath_to_root.push_back(x);
            x = parent[x];
        }
        return xpath_to_root;
    }

    // Žiedo susitraukimas
    // c - id naujo susitraukiusio žiedo
    // x, y - virsunes brauna tarp kuriu susidaro cikla
    // vx, vy - keliai į šakni nuo x ir y atitinkamai
    // r - žemiausias bendras protėvis
    // Sudetingumas O(n*|#blossom|) kur |blossom| virsuniu/žiedu sutrauktu į c skaicius
    
    void shrinking(int blossom_, int vertex_x, int vertex_y, vector<int>& xpath_to_root, vector<int>& ypath_to_root)
    
    {
        vertices_in_blossom[blossom_].clear();
        int r = xpath_to_root.back();
        while (!xpath_to_root.empty() && !ypath_to_root.empty() && xpath_to_root.back() == ypath_to_root.back()) 
        {
            r = xpath_to_root.back();
            xpath_to_root.pop_back();
            ypath_to_root.pop_back();
        }
        vertices_in_blossom[blossom_].push_back(r);
        vertices_in_blossom[blossom_].insert(vertices_in_blossom[blossom_].end(), xpath_to_root.rbegin(), xpath_to_root.rend());
        vertices_in_blossom[blossom_].insert(vertices_in_blossom[blossom_].end(), ypath_to_root.begin(), ypath_to_root.end());
        for (int i = 0; i <= blossom_; i++)
        {
            edge_matrix[blossom_][i] = edge_matrix[i][blossom_] = -1;
        }
        for (int z : vertices_in_blossom[blossom_])
        {
            blossom_containing_vertex[z] = blossom_;
            for (int i = 0; i < blossom_; i++) 
            {
                if (edge_matrix[z][i] != -1) {
                    edge_matrix[blossom_][i] = z;
                    edge_matrix[i][blossom_] = edge_matrix[i][z];
                }
            }
        }
    }

    // Kelio atstatymas (tai yra visu susitraukiusiu žiedu atstatymas)
    // z - stako virsune
    // w - sekantis elementas uz z
    // i - vertices_in_blossom[z] vektoriaus paskutinios virsunes indeksas
    // j - vertices_in_blossom[z] vektoriaus pirmos virsunes indeksas
    // dif - kryptis kuria turime eiti nuo i iki j kad kelias teisingai alternuotusi
    // Sudetingumas O(n)
    vector<int> lift(vector<int>& xpath_to_root) 
    {
        vector<int> A;
        while (xpath_to_root.size() >= 2) 
        {
            int top_of_stack = xpath_to_root.back(); xpath_to_root.pop_back();
            if (top_of_stack < n) 
            {
                A.push_back(top_of_stack);
                continue;
            }
            int behind_the_top_of_stack = xpath_to_root.back();
            int i = (A.size() % 2 == 0 ? find(vertices_in_blossom[top_of_stack].begin(), vertices_in_blossom[top_of_stack].end(), edge_matrix[top_of_stack][behind_the_top_of_stack]) - vertices_in_blossom[top_of_stack].begin() : 0);
            int j = (A.size() % 2 == 1 ? find(vertices_in_blossom[top_of_stack].begin(), vertices_in_blossom[top_of_stack].end(), edge_matrix[top_of_stack][A.back()]) - vertices_in_blossom[top_of_stack].begin() : 0);
            int k = vertices_in_blossom[top_of_stack].size();
            int direction = (A.size() % 2 == 0 ? i % 2 == 1 : j % 2 == 0) ? 1 : k - 1;
            while (i != j) 
            {
                xpath_to_root.push_back(vertices_in_blossom[top_of_stack][i]);
                i = (i + direction) % k;
            }
            xpath_to_root.push_back(vertices_in_blossom[top_of_stack][i]);
        }
        return A;
    }



    int find_maximum_matching() 
    {
        for (int maximum_match_size = 0; ; maximum_match_size++) 
        {
            fill(label.begin(), label.end(), 0);   // uzpildom label vektori nuliais
            queue<int> Queue;
            for (int i = 0; i < m; i++) blossom_containing_vertex[i] = i;
            for (int i = 0; i < n; i++) 
            {
                if (mate[i] == -1) 
                {
                    Queue.push(i);
                    parent[i] = i;
                    label[i] = 1;
                }
            }
            int blossom_ = n;
            bool augmenting_path_exists = false;
            while (!Queue.empty() && !augmenting_path_exists)
            {
                int vertex_x = Queue.front(); Queue.pop();
                if (blossom_containing_vertex[vertex_x] != vertex_x) 
                    continue;
                for (int vertex_y = 0; vertex_y < blossom_; vertex_y++)
                {
                    if (blossom_containing_vertex[vertex_y] == vertex_y && edge_matrix[vertex_x][vertex_y] != -1) 
                    {
                        if (label[vertex_y] == 0) 
                        {
                            parent[vertex_y] = vertex_x;
                            label[vertex_y] = 2;
                            parent[mate[vertex_y]] = vertex_y;
                            label[mate[vertex_y]] = 1;
                            Queue.push(mate[vertex_y]);
                        }
                        else if (label[vertex_y] == 1) 
                        {
                            vector<int> xpath_to_root = path_to_root(vertex_x);
                            vector<int> ypath_to_root = path_to_root(vertex_y);
                            if (xpath_to_root.back() == ypath_to_root.back())
                            {
                                shrinking(blossom_, vertex_x, vertex_y, xpath_to_root, ypath_to_root);
                                Queue.push(blossom_);
                                parent[blossom_] = parent[vertices_in_blossom[blossom_][0]];
                                label[blossom_] = 1;
                                blossom_++;
                            }
                            else 
                            {
                                augmenting_path_exists = true;
                                xpath_to_root.insert(xpath_to_root.begin(), vertex_y);
                                ypath_to_root.insert(ypath_to_root.begin(), vertex_x);
                                vector<int> A = lift(xpath_to_root);
                                vector<int> B = lift(ypath_to_root);
                                A.insert(A.end(), B.rbegin(), B.rend());
                                for (int i = 0; i < (int)A.size(); i += 2) {
                                    match(A[i], A[i + 1]);
                                    if (i + 2 < (int)A.size()) add_edge(A[i + 1], A[i + 2]);
                                }
                            }
                            break;
                        }
                    }
                }
            }
            if (!augmenting_path_exists) 
            {
                return maximum_match_size;
            }
        }
    }

};


//                   

int main()
{

    int random_1;
    int random_2;
    int maximum_number_of_neighbours;
    int CASE;
    int number_of_vertices, number_of_edges{ 0 };

    while(true)
    
    {
        cout << "Jeigu norite ivesti grafa, spauskite 1 ENTER" << endl;
        cout << "Jeigu norite atsitiktinai sugeneruoti grafa spauskite 2 ENTER" << endl;
        cout << "Jeigu norite atlikti jau paruosta Edmonso algoritmo spartos testavima spauskite 3 ENTER" << endl;
        cin >> CASE;
        if (CASE == 1)
        {
            int u = 1, v=1;
            cout << "Iveskite grafo virsuniu skaiciu : " << endl;
            cin >> number_of_vertices;
            Grafas grafas(number_of_vertices);
            cout << "Iveskite grafo briaunas : " << endl;
            cout << "Norint baigti spauskite 0 ENTER" << endl;
            while(true)
            {
                cin >> u;

                if (u == 0) break;

                cin >> v;
                
                if (v == 0) break;

                grafas.add_edge(u, v);

                number_of_edges++;
            }


            auto start = high_resolution_clock::now();

            cout << "Grafo maximalaus poravimo dydis yra " << grafas.find_maximum_matching() << endl;
            for (int i = 0; i < number_of_vertices; i++)
            {
                if (i < grafas.mate[i]) {
                    cout << i << ' ' << grafas.mate[i] << endl;

                }
            }
            cout << endl;
            auto stop = high_resolution_clock::now();
            double duration = duration_cast<microseconds>(stop - start).count();
            cout << "Grafo, turincio " << number_of_vertices << " virsuniu ir " << number_of_edges << " briaunu Edmondso algoritmas truko " << duration / 1000000 << "sekundžiu" << endl << endl << endl;
            cout << "Norint Baigti spauskite 0 ENTER, norite testi, spauskite 1 " << endl;


        }

        else if (CASE == 2)
        {
            cout << "Iveskite briaunu skaiciu : ";
            cin >> number_of_vertices;
            cout << "Iveskite kiek briaunu norite kad turetu kiekviena virsune : ";
            cin >> maximum_number_of_neighbours;



            number_of_edges = 0;
            Grafas grafas(number_of_vertices);
            cout << "Sugeneruotas grafas" << endl;
            for (int i = 0; i != number_of_vertices; i++)
            {
                for (int a = 0; a != maximum_number_of_neighbours; a++)
                {
                    random_2 = rand() % number_of_vertices + 1;
                    grafas.add_edge(i + 1, random_2);
                    number_of_edges++;
                    cout << i + 1 << " " << random_2 << endl;
                }
            }
            cout << endl;


            auto start = high_resolution_clock::now();

            cout << "Grafo maximalaus poravimo dydis yra " << grafas.find_maximum_matching() << endl;
            for (int i = 0; i < number_of_vertices; i++)
            {
                if (i < grafas.mate[i]) {
                    cout << i << ' ' << grafas.mate[i] << endl;

                }
            }
            cout << endl;
            auto stop = high_resolution_clock::now();
            double duration = duration_cast<microseconds>(stop - start).count();
            cout << "Grafo, turincio " << number_of_vertices << " virsuniu ir " << number_of_edges << " briaunu Edmondso algoritmas truko " << duration / 1000000 << "sekundžiu" << endl << endl << endl;
            cout << "Norint Baigti spauskite 0 ENTER, norite testi, spauskite 1 " << endl;

        }

        else if (CASE == 3)
        {
            cout << "-------------------Istirsime spartos priklausomybe nuo virsuniu skaiciaus---------------------------" << endl;
            maximum_number_of_neighbours = 15;
            for (int k = 1; k != 5; k++)
            {
                number_of_vertices = 1000 * k;
                Grafas grafas(number_of_vertices);
                for (int i = 0; i != number_of_vertices; i++)
                {
                    for (int a = 0; a != maximum_number_of_neighbours; a++)
                    {
                        random_2 = rand() % number_of_vertices + 1;
                        grafas.add_edge(i + 1, random_2);
                        number_of_edges++;
                    }
                }
                cout << "Sugeneruotas grafas. Vyksta poravimas..." << endl;
                cout << endl;


                auto start = high_resolution_clock::now();

                cout << "Grafo maximalaus poravimo dydis yra " << grafas.find_maximum_matching() << endl;
                for (int i = 0; i < number_of_vertices; i++)
                {
                    if (i < grafas.mate[i]) {

                    }
                }
                cout << endl;
                auto stop = high_resolution_clock::now();
                double duration = duration_cast<microseconds>(stop - start).count();
                cout << "Grafo, turincio " << number_of_vertices << " virsuniu ir " << number_of_edges << " briaunu Edmondso algoritmas truko " << duration / 1000000 << "sekundžiu" << endl << endl << endl;
            }

            cout << "-------------------Istirsime spartos priklausomybe nuo briaunu skaiciaus---------------------------" << endl;

            number_of_vertices = 3000;
            maximum_number_of_neighbours = 10;
            for(int k = 1; k!= 5;k++)
            {
                Grafas grafas(number_of_vertices);
                for (int i = 0; i != number_of_vertices; i++)
                {
                    for (int a = 0; a != 100 * k; a++)
                    {
                        random_2 = rand() % number_of_vertices + 1;
                        grafas.add_edge(i + 1, random_2);
                        number_of_edges++;
                    }
                }
                cout << "Sugeneruotas grafas. Vyksta poravimas..." << endl;
                cout << endl;


                auto start = high_resolution_clock::now();

                cout << "Grafo maximalaus poravimo dydis yra " << grafas.find_maximum_matching() << endl;
                for (int i = 0; i < number_of_vertices; i++)
                {
                    if (i < grafas.mate[i]) {
                    }
                }
                cout << endl;
                auto stop = high_resolution_clock::now();
                double duration = duration_cast<microseconds>(stop - start).count();
                cout << "Grafo, turincio " << number_of_vertices << " virsuniu ir " << number_of_edges << " briaunu Edmondso algoritmas truko " << duration / 1000000 << "sekundžiu" << endl << endl << endl;

            }
            cout << "Testavimas baigtas" << endl;
        }

        int baigti;
        cin >> baigti;
        if (baigti == 0)
        {
            break;
        }
    }
        return 0;
}
