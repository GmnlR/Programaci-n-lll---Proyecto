#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

using namespace std;

// Estructura para almacenar información de la película
struct Pelicula {
    string imdb_id;
    string titulo;
    string sinopsis;
    vector<string> tags;
    string split;
    string fuente_sinopsis;
};

// Función para leer el archivo CSV
vector<Pelicula> leerCSV(const string& nombreArchivo) {
    vector<Pelicula> peliculas;
    ifstream archivo(nombreArchivo);
    string linea;

    if (!archivo.is_open()) {
        throw runtime_error("No se puede abrir el archivo " + nombreArchivo);
    }

    // Saltar la primera línea (cabecera)
    getline(archivo, linea);

    while (getline(archivo, linea)) {
        stringstream ss(linea);
        Pelicula pelicula;
        getline(ss, pelicula.imdb_id, ',');
        getline(ss, pelicula.titulo, ',');
        getline(ss, pelicula.sinopsis, ',');

        string tags_str;
        getline(ss, tags_str, ',');
        istringstream tags_ss(tags_str);
        string tag;
        while (getline(tags_ss, tag, '|')) {
            pelicula.tags.push_back(tag);
        }

        getline(ss, pelicula.split, ',');
        getline(ss, pelicula.fuente_sinopsis, ',');

        peliculas.push_back(pelicula);
    }

    return peliculas;
}

// Funciones para indexar películas
unordered_map<string, Pelicula> indexarPeliculasPorTitulo(const vector<Pelicula>& peliculas) {
    unordered_map<string, Pelicula> indice;
    for (const auto& pelicula : peliculas) {
        indice[pelicula.titulo] = pelicula;
    }
    return indice;
}

unordered_map<string, vector<Pelicula>> indexarPeliculasPorTag(const vector<Pelicula>& peliculas) {
    unordered_map<string, vector<Pelicula>> indice;
    for (const auto& pelicula : peliculas) {
        for (const auto& tag : pelicula.tags) {
            indice[tag].push_back(pelicula);
        }
    }
    return indice;
}

// Funciones de búsqueda
vector<Pelicula> buscarPorTitulo(const unordered_map<string, Pelicula>& indiceTitulo, const string& palabra) {
    vector<Pelicula> resultados;
    for (const auto& [titulo, pelicula] : indiceTitulo) {
        if (titulo.find(palabra) != string::npos) {
            resultados.push_back(pelicula);
        }
    }
    return resultados;
}

vector<Pelicula> buscarPorTag(const unordered_map<string, vector<Pelicula>>& indiceTag, const string& tag) {
    if (indiceTag.find(tag) != indiceTag.end()) {
        return indiceTag.at(tag);
    }
    return {};
}

// Función para mostrar las películas y la sinopsis
void mostrarPeliculas(const vector<Pelicula>& peliculas, int inicio, int cantidad) {
    for (int i = inicio; i < inicio + cantidad && i < peliculas.size(); ++i) {
        cout << i + 1 << ". " << peliculas[i].titulo << endl;
    }
}

void mostrarSinopsis(const Pelicula& pelicula) {
    cout << "Sinopsis: " << pelicula.sinopsis << endl;
    cout << "[1] Like\n[2] Ver mas tarde\n";
}

// Función para recomendar películas
vector<Pelicula> recomendarPeliculas(const vector<Pelicula>& peliculas, const unordered_set<string>& likes) {
    vector<Pelicula> recomendaciones;
    // Algoritmo simple basado en tags comunes
    for (const auto& pelicula : peliculas) {
        for (const auto& tag : pelicula.tags) {
            if (likes.find(tag) != likes.end()) {
                recomendaciones.push_back(pelicula);
                break;
            }
        }
    }
    return recomendaciones;
}

int main() {
    try {
        string nombreArchivo = "C:\\Users\\HP\\CLionProjects\\ProyectoProgra\\Programacion-lll---Proyecto\\mpst_full_data.csv";
        auto peliculas = leerCSV(nombreArchivo);

        auto indiceTitulo = indexarPeliculasPorTitulo(peliculas);
        auto indiceTag = indexarPeliculasPorTag(peliculas);

        unordered_set<string> peliculasLike;
        unordered_set<string> peliculasVerMasTarde;

        while (true) {
            cout << "Buscar pelicula por [1] Titulo o [2] Tag: ";
            int opcionBusqueda;
            cin >> opcionBusqueda;

            vector<Pelicula> resultados;
            if (opcionBusqueda == 1) {
                cout << "Ingrese la palabra o frase: ";
                string palabra;
                cin.ignore();
                getline(cin, palabra);
                resultados = buscarPorTitulo(indiceTitulo, palabra);
            } else if (opcionBusqueda == 2) {
                cout << "Ingrese el tag: ";
                string tag;
                cin.ignore();
                getline(cin, tag);
                resultados = buscarPorTag(indiceTag, tag);
            } else {
                cout << "Opcion no valida" << endl;
                continue;
            }

            mostrarPeliculas(resultados, 0, 5);

            int opcionVer;
            cout << "Seleccione una pelicula para ver la sinopsis (0 para salir): ";
            cin >> opcionVer;

            if (opcionVer == 0) break;

            if (opcionVer > 0 && opcionVer <= resultados.size()) {
                mostrarSinopsis(resultados[opcionVer - 1]);
                int opcionAccion;
                cin >> opcionAccion;

                if (opcionAccion == 1) {
                    peliculasLike.insert(resultados[opcionVer - 1].titulo);
                } else if (opcionAccion == 2) {
                    peliculasVerMasTarde.insert(resultados[opcionVer - 1].titulo);
                }
            } else {
                cout << "Opcion no valida" << endl;
            }
        }

        auto recomendaciones = recomendarPeliculas(peliculas, peliculasLike);
        cout << "Recomendaciones basadas en tus Likes: " << endl;
        mostrarPeliculas(recomendaciones, 0, 5);

        cout << "Peliculas en Ver mas tarde: " << endl;
        for (const auto& titulo : peliculasVerMasTarde) {
            cout << titulo << endl;
        }

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
