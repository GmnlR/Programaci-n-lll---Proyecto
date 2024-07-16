#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>

using namespace std;

// Estructura para almacenar informacion de la pelicula
struct Pelicula {
    string titulo;
    string sinopsis;
    vector<string> tags;
    string split;
    string fuente_sinopsis;
};

// Clase que maneja la carga y búsqueda de peliculas
class GestorPeliculas {
public:
    GestorPeliculas(const string& nombreArchivo) {
        peliculas = leerCSV(nombreArchivo);
        indexarPeliculas();
    }

    vector<Pelicula*> buscarPorTituloOSinopsis(const string& busqueda) {
        vector<Pelicula*> resultados;
        unordered_set<string> titulosEncontrados;

        string busquedaMinusculas = toLower(busqueda);

        // Buscar coincidencias exactas en los titulos
        for (const auto& par : indiceTitulo) {
            string tituloMinusculas = toLower(par.first);

            if (tituloMinusculas == busquedaMinusculas) {
                resultados.push_back(par.second.get());
                titulosEncontrados.insert(par.first);
            }
        }

        // Buscar coincidencias en las sinopsis
        for (const auto& pelicula : peliculas) {
            string sinopsisMinusculas = toLower(pelicula.sinopsis);

            if (sinopsisMinusculas.find(busquedaMinusculas) != string::npos) {
                if (titulosEncontrados.find(pelicula.titulo) == titulosEncontrados.end()) {
                    resultados.push_back(const_cast<Pelicula*>(&pelicula));
                    titulosEncontrados.insert(pelicula.titulo);
                }
            }
        }

        return resultados;
    }

    vector<Pelicula*> buscarPorTag(const string& tag) {
        auto it = indiceTag.find(tag);
        if (it != indiceTag.end()) {
            return it->second;
        }
        return {};
    }

    const vector<Pelicula>& getPeliculas() const {
        return peliculas;
    }

private:
    vector<Pelicula> peliculas;
    unordered_map<string, unique_ptr<Pelicula>> indiceTitulo;
    unordered_map<string, vector<Pelicula*>> indiceTag;

    vector<Pelicula> leerCSV(const string& nombreArchivo) {
        vector<Pelicula> peliculas;
        ifstream archivo(nombreArchivo);
        string linea;

        if (!archivo.is_open()) {
            throw runtime_error("No se puede abrir el archivo" + nombreArchivo);
        }

        getline(archivo, linea); // Saltar la primera línea (cabecera)

        while (getline(archivo, linea)) {
            stringstream ss(linea);
            Pelicula pelicula;

            getline(ss, pelicula.titulo, ';');
            getline(ss, pelicula.sinopsis, ';');
            string tags_str;
            getline(ss, tags_str, ';');
            pelicula.tags = splitString(tags_str, ',');
            getline(ss, pelicula.split, ';');
            getline(ss, pelicula.fuente_sinopsis, ';');

            peliculas.push_back(pelicula);
        }

        return peliculas;
    }

    vector<string> splitString(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    void indexarPeliculas() {
        for (auto& pelicula : peliculas) {
            indiceTitulo[pelicula.titulo] = make_unique<Pelicula>(pelicula);
            for (const auto& tag : pelicula.tags) {
                indiceTag[tag].push_back(&pelicula);
            }
        }
    }

    string toLower(const string& str) {
        string result = str;
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
};

// Clase que maneja la interfaz del usuario
class InterfazUsuario {
public:
    InterfazUsuario(GestorPeliculas& gestor) : gestor(gestor) {}

    void iniciar() {
        mostrarPeliculasVerMasTarde();
        while (true) {
            cout << "\n[1] Buscar pelicula\n[2] Ver peliculas en 'Ver mas tarde'\n[3] Ver recomendaciones\n[0] Salir\nSeleccione una opcion: ";
            int opcionPrincipal;
            cin >> opcionPrincipal;

            if (opcionPrincipal == 0) break;

            switch (opcionPrincipal) {
                case 1:
                    buscarPelicula();
                    break;
                case 2:
                    mostrarPeliculasVerMasTarde();
                    break;
                case 3:
                    mostrarRecomendaciones();
                    break;
                default:
                    cout << "Opcion no valida\n";
                    break;
            }
        }
    }

private:
    GestorPeliculas& gestor;
    unordered_set<string> peliculasLike;
    unordered_set<string> peliculasVerMasTarde;

    void buscarPelicula() {
        cout << "\nBuscar pelicula por [1] Titulo o frase o [2] Tag: ";
        int opcionBusqueda;
        cin >> opcionBusqueda;

        vector<Pelicula*> resultados;
        if (opcionBusqueda == 1) {
            cout << "Ingrese el titulo de la pelicula: ";
            string titulo;
            cin.ignore();
            getline(cin, titulo);
            resultados = gestor.buscarPorTituloOSinopsis(titulo);
        } else if (opcionBusqueda == 2) {
            cout << "Ingrese el tag: ";
            string tag;
            cin.ignore();
            getline(cin, tag);
            resultados = gestor.buscarPorTag(tag);
        } else {
            cout << "Opcion no valida\n";
            return;
        }

        if (resultados.empty()) {
            cout << "El dato ingresado no existe en nuestra base de datos.\n";
            return;
        }

        int inicio = 0;
        while (true) {
            mostrarPeliculasPaginadas(resultados, inicio);

            if (inicio + 5 >= static_cast<int>(resultados.size())) {
                break;
            }

            cout << "\nPresiona 'n' para ver las siguientes 5 peliculas o cualquier otra tecla para seleccionar una: ";
            char opcionVerMas;
            cin >> opcionVerMas;

            if (opcionVerMas == 'n') {
                inicio += 5;
            } else {
                break;
            }
        }

        int opcionVer;
        cout << "\nSeleccione una pelicula para ver la sinopsis (0 para salir): ";
        cin >> opcionVer;

        if (opcionVer == 0) return;

        if (opcionVer > 0 && opcionVer <= static_cast<int>(resultados.size())) {
            mostrarSinopsis(*resultados[opcionVer - 1]);
        } else {
            cout << "Opcion no valida\n";
        }
    }

    void mostrarPeliculasPaginadas(const vector<Pelicula*>& peliculas, int inicio) {
        int limite = min(static_cast<int>(peliculas.size()), inicio + 5);

        for (int i = inicio; i < limite; ++i) {
            cout << i + 1 << ". " << peliculas[i]->titulo << endl;
        }
    }

    void mostrarSinopsis(const Pelicula& pelicula) {
        cout << "\nSinopsis: " << pelicula.sinopsis << endl;
        cout << "[1] Like   [2] Ver mas tarde   [0] Volver al menu principal: ";

        int opcion;
        cin >> opcion;

        switch (opcion) {
            case 1:
                peliculasLike.insert(pelicula.titulo);
                break;
            case 2:
                peliculasVerMasTarde.insert(pelicula.titulo);
                break;
            default:
                break;
        }
    }

    void mostrarPeliculasVerMasTarde() {
        if (!peliculasVerMasTarde.empty()) {
            cout << "\nPeliculas en Ver mas tarde:\n";
            for (const auto& titulo : peliculasVerMasTarde) {
                cout << titulo << endl;
            }
        } else {
            cout << "\nNo hay peliculas en Ver mas tarde.\n";
        }
    }

    void mostrarRecomendaciones() {
        auto recomendaciones = recomendarPeliculas(gestor.getPeliculas(), peliculasLike);
        if (!recomendaciones.empty()) {
            cout << "\nRecomendaciones basadas en tus Likes:\n";
            mostrarPeliculasPaginadas(recomendaciones, 0);
        } else {
            cout << "\nNo hay recomendaciones en este momento.\n";
        }
    }

    vector<Pelicula*> recomendarPeliculas(const vector<Pelicula>& peliculas, const unordered_set<string>& likes) {
        vector<Pelicula*> recomendaciones;
        unordered_set<string> tagsLikes;

        for (const auto& pelicula : peliculas) {
            if (likes.find(pelicula.titulo) != likes.end()) {
                for (const auto& tag : pelicula.tags) {
                    tagsLikes.insert(tag);
                }
            }
        }

        for (const auto& pelicula : peliculas) {
            bool tieneTag = false;
            for (const auto& tag : pelicula.tags) {
                if (tagsLikes.find(tag) != tagsLikes.end()) {
                    tieneTag = true;
                    break;
                }
            }
            if (tieneTag && likes.find(pelicula.titulo) == likes.end()) {
                recomendaciones.push_back(const_cast<Pelicula*>(&pelicula));
            }
        }

        return recomendaciones;
    }
};

int main() {
    try {
        string nombreArchivo = "C:\\Users\\HP\\CLionProjects\\proyec_editbyangeline\\mpst_full_data_puntoycoma22.csv";
        GestorPeliculas gestor(nombreArchivo);
        InterfazUsuario interfaz(gestor);
        interfaz.iniciar();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}