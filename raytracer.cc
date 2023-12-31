#include "geometry/geometry.h"
#include "sdlutils.h"
#include <algorithm>
#include "vector"

// Die folgenden Kommentare beschreiben Datenstrukturen und Funktionen
// Die Datenstrukturen und Funktionen die weiter hinten im Text beschrieben sind,
// hängen höchstens von den vorhergehenden Datenstrukturen ab, aber nicht umgekehrt.

// Eine "Kamera", die von einem Augenpunkt aus in eine Richtung senkrecht auf ein Rechteck (das Bild) zeigt.
// Für das Rechteck muss die Auflösung oder alternativ die Pixelbreite und -höhe bekannt sein.
// Für ein Pixel mit Bildkoordinate kann ein Sehstrahl erzeugt werden.


// Für die "Farbe" benötigt man nicht unbedingt eine eigene Datenstruktur.
// Sie kann als Vector3df implementiert werden mit Farbanteil von 0 bis 1.
// Vor Setzen eines Pixels auf eine bestimmte Farbe (z.B. 8-Bit-Farbtiefe),
// kann der Farbanteil mit 255 multipliziert// kann der Farbanteil mit 255 multipliziert  und der Nachkommaanteil verworfen werden.  und der Nachkommaanteil verworfen werden.
using Color = Vector3df;


// Das "Material" der Objektoberfläche mit ambienten, diffusem und reflektiven Farbanteil.

// Ein "Objekt", z.B. eine Kugel oder ein Dreieck, und dem zugehörigen Material der Oberfläche.
// Im Prinzip ein Wrapper-Objekt, das mindestens Material und geometrisches Objekt zusammenfasst.
// Kugel und Dreieck finden Sie in geometry.h/tcc

struct Object {
    Sphere3df sphere;
    Color color;
    bool is_reflective;
};

using Scene = std::vector<Object>;

// verschiedene Materialdefinition, z.B. Mattes Schwarz, Mattes Rot, Reflektierendes Weiss, ...
// im wesentlichen Variablen, die mit Konstruktoraufrufen initialisiert werden.


// Die folgenden Werte zur konkreten Objekten, Lichtquellen und Funktionen, wie Lambertian-Shading
// oder die Suche nach einem Sehstrahl für das dem Augenpunkt am nächsten liegenden Objekte,
// können auch zusammen in eine Datenstruktur für die gesammte zu
// rendernde "Szene" zusammengefasst werden.

// Die Cornelbox aufgebaut aus den Objekten
// Am besten verwendet man hier einen std::vector< ... > von Objekten.

// Punktförmige "Lichtquellen" können einfach als Vector3df implementiert werden mit weisser Farbe,
// bei farbigen Lichtquellen müssen die entsprechenden Daten in Objekt zusammengefaßt werden
// Bei mehreren Lichtquellen können diese in einen std::vector gespeichert werden.

// Sie benötigen eine Implementierung von Lambertian-Shading, z.B. als Funktion
// Benötigte Werte können als Parameter übergeben werden, oder wenn diese Funktion eine Objektmethode eines
// Szene-Objekts ist, dann kann auf die Werte teilweise direkt zugegriffen werden.
// Bei mehreren Lichtquellen muss der resultierende diffuse Farbanteil durch die Anzahl Lichtquellen geteilt werden.

// Für einen Sehstrahl aus allen Objekte, dasjenige finden, das dem Augenpunkt am nächsten liegt.
// Am besten einen Zeiger auf das Objekt zurückgeben. Wenn dieser nullptr ist, dann gibt es kein sichtbares Objekt.

// Die rekursive raytracing-Methode. Am besten ab einer bestimmten Rekursionstiefe (z.B. als Parameter übergeben) abbrechen.

Sphere3df left_wall = {{-10021.0f, 0.f, 0.0f}, 10000.0f};
Sphere3df right_wall = {{10021.0f, 0.f, 0.0f}, 10000.0f};
Sphere3df ceiling = {{0.0f, 10012.f, 0.0f}, 10000.0f};
Sphere3df ground = {{0.0f, -10012.f, 0.0f}, 10000.0f};
Sphere3df back_wall = {{0.f, 0.f, -10030.f}, 10000.0f};

Sphere3df sphere1 = {{0.f, -8.f, -17.f}, 3.f};
Sphere3df sphere2 = {{10.f, -8.f, -14.f}, 3.f};
Sphere3df sphere3 = {{0.f, -8.f, -30.f}, 3.f};
Sphere3df sphere4 = {{-10.f, 5.f, -5.f}, 5.f};
Sphere3df sphere5 = {{-7.f, -6.f, -17.f}, 2.f};

Scene cornell_box = {
        {sphere1,    Color{0.f, 1.f, 1.f},   true},
        {sphere2,    Color{1.f, 0.f, 0.f},   false},
        {sphere3,    Color{0.2f, 0.5f, 1.f}, false},
        {sphere4,    Color{1.f, 0.5f, 1.f},  true},
        {sphere5,    Color{1.f, 0.4f, .3f},  false},
        {left_wall,  Color{1.f, 0.f, 0.f},   false},
        {right_wall, Color{0.f, 1.f, 0.f},   false},
        {ceiling,    Color{1.f, 1.f, 1.f},   false},
        {ground,     Color{1.f, 1.f, 1.f},   false},
        {back_wall,  Color{1.f, 1.f, 1.f},   false},
};

Vector3df light_source = {5.f, 7.f, -16.f};

Object find_nearest_object(Ray3df &ray, Scene &scene) {
    Object nearest_obj = scene[0];
    float min_t = INFINITY;
    for (Object obj: scene) {
        float t = obj.sphere.intersects(ray);
        if (t > 0 && t < min_t) {
            min_t = t;
            nearest_obj = obj;
        }
    }
    return nearest_obj;
};

Intersection_Context<float, 3u> get_clean_intersection_context() {
    Intersection_Context<float, 3u> intersection_context;
    intersection_context.intersection = {0.f, 0.f, 0.f};
    intersection_context.normal = {0.f, 0.f, 0.f};
    intersection_context.t = 0.f;
    intersection_context.u = 0.f;
    intersection_context.v = 0.f;
    return intersection_context;
};


Color lambertian(Ray3df &ray, Vector3df &light, Scene &scene, Object &nearest_obj) {
    Intersection_Context<float, 3u> intersection_context = get_clean_intersection_context();
    nearest_obj.sphere.intersects(ray, intersection_context);
    Vector3df normal = intersection_context.normal;
    Vector3df intersection_point = intersection_context.intersection + (0.015f * normal);
    Vector3df light_direction = light - intersection_point;

    // ray hits object at a point
    // if light is blocked, the point is dark
    // light is blocked when there is an object between the intersection point and the light source
    // if the distance of the intersection ray is greater than point and light source the light is not blocked
    float distance_to_light = light_direction.length();
    Ray3df from_intersection_to_light = {intersection_point, light_direction};
    for (Object obj: scene) {
        from_intersection_to_light.direction.normalize();
        // t is now the distance of the ray
        float t = obj.sphere.intersects(from_intersection_to_light);
        if (t > 0 && t <= distance_to_light) return {0.f, 0.f, 0.f};
    }

    // calculate lambertian shading
    light_direction.normalize();
    normal.normalize();
    float cos_theta = std::max<float>(0, light_direction * normal);
    return nearest_obj.color * cos_theta;
}


Color trace(Ray3df &ray, Scene &scene, int max_depth) {
    Object nearest_obj = find_nearest_object(ray, scene);
    if (!nearest_obj.is_reflective || max_depth == 0) {
        return lambertian(ray, light_source, scene, nearest_obj);
    }

    Intersection_Context<float, 3u> context = get_clean_intersection_context();
    nearest_obj.sphere.intersects(ray, context);

    Vector3df n = context.normal;
    n.normalize();
    Vector3df v = ray.direction;
    Vector3df reflective_dir = v - 2 * (v * n) * n;
    Vector3df intersection_point = context.intersection + (0.1f * n);
    Ray3df reflective{intersection_point, reflective_dir};
    return trace(reflective, scene, max_depth - 1);
}

int main() {
    // Ein "Bildschirm", der das Setzen eines Pixels kapselt
    // Der Bildschirm hat eine Auflösung (Breite x Höhe)
    // Kann zur Ausgabe einer PPM-Datei verwendet werden oder
    // mit SDL2 implementiert werden.
    float aspect_ratio = 16.0f / 9.0f;
    int width = 800;
    int height = static_cast<int>(static_cast<float>(width) / aspect_ratio);
    height = (height < 1) ? 1 : height;

    // Camera
    float focal_length = 1.f;
    float viewport_height = 2.0f;
    float viewport_width = viewport_height *
                           (static_cast<float>(width) / static_cast<float>(height));
    auto camera_center = Vector3df{0.0, 0.0, 0.0};
// Calculate vectors across horizontal and vertical viewport
    Vector3df viewport_x = Vector3df{viewport_width, 0.0, 0.0};
    Vector3df viewport_y = Vector3df{0.0, -viewport_height, 0.0};

    Vector3df pixel_delta_x = viewport_x * (1.0f / static_cast<float>(width));
    Vector3df pixel_delta_y = viewport_y * (1.0f / static_cast<float>(height));

    Vector3df viewport_upper_left = camera_center
                                    - Vector3df{0.0, 0.0, focal_length}
                                    - (viewport_x * 0.5f)
                                    - (viewport_y * 0.5f);
    // center of the upper left pixel
    Vector3df pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_x + pixel_delta_y);

    sdlutils::sdl("Ray Tracer", width, height);
    sdlutils::loop();
    //std::cout << "P3\n" << width << ' ' << height << "\n255\n";
    const int num_samples = 4;
    for (int y = 0; y < height; y++) {
        std::clog << "\rScanlines remaining: " << (height - y) << ' ' << std::flush;
        for (int x = 0; x < width; x++) {

            // Super sampling
            Color accumulated_color{0.0f, 0.0f, 0.0f};
            for (int s = 0; s < num_samples; s++) {
                float subpixel_x =
                        static_cast<float>(x) + static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                float subpixel_y =
                        static_cast<float>(y) + static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                Vector3df pixel_center = pixel00_loc
                                         + (subpixel_x * pixel_delta_x)
                                         + (subpixel_y * pixel_delta_y);
                Vector3df ray_direction = pixel_center - camera_center;
                Ray3df r{camera_center, ray_direction};
                accumulated_color += trace(r, cornell_box, 3);
            }
            Color color{accumulated_color[0] / num_samples,
                        accumulated_color[1] / num_samples,
                        accumulated_color[2] / num_samples};
            int ir = static_cast<int>(255 * color[0]);
            int ig = static_cast<int>(255 * color[1]);
            int ib = static_cast<int>(255 * color[2]);
            //std::cout << ir << ' ' << ig << ' ' << ib << '\n';
            sdlutils::setDrawColor(sdlutils::createColor(ir, ig, ib, 255));
            sdlutils::drawPoint(x, y);
        }
    }
    std::clog << "\rDone.                 \n";
    while (sdlutils::running) {
        sdlutils::loop();
    }
    return 0;

    // Kamera erstellen

    // Für jede Pixelkoordinate x,y
    //   Sehstrahl für x,y mit Kamera erzeugen
    //   voidFarbe mit raytracing-Methode bestimmen
    //   Beim Bildschirm die Farbe für Pixel x,y, setzten
}

