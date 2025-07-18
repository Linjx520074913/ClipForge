import { Request, Response, Router } from 'express';
import { useMockData } from './mock-data';

const router = Router();

const paths = {
    query: '/query'
};

/**
 * 查询滤镜列表
 */
router.get(paths.query, async(req: Request, res:Response) => {
    const { shaders } = useMockData();
    res.json(shaders);
});



// **** Export default **** //

export default router;
